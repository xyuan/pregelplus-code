#include <vector>
#include <string>
#include <fstream>
#include <cstdlib>
#include <graphlab.hpp>

typedef int tag_type;

//webuk SOURCE = 44881114;
//webbase 14855206
int SOURCE;
const int DEST = -1;

struct vertex_data : graphlab::IS_POD_TYPE {
    tag_type tag;
    vertex_data(tag_type tag = 0)
        : tag(tag)
    {
    }
};

typedef graphlab::empty edge_data;
typedef graphlab::distributed_graph<vertex_data, edge_data> graph_type;

inline graph_type::vertex_type get_other_vertex(
    const graph_type::edge_type& edge,
    const graph_type::vertex_type& vertex)
{
    return vertex.id() == edge.source().id() ? edge.target() : edge.source();
}

struct sum_tag_type : graphlab::IS_POD_TYPE {
    tag_type tag;
    sum_tag_type(tag_type tag = 0)
        : tag(tag)
    {
    }
    sum_tag_type& operator+=(const sum_tag_type& other)
    {
        tag |= other.tag;
        return *this;
    }
};

// gather type is graphlab::empty, then we use message model
class reach : public graphlab::ivertex_program<graph_type, graphlab::empty,
                                               sum_tag_type>,
              public graphlab::IS_POD_TYPE {
    bool changed;
    tag_type sum_tag;

public:
    void init(icontext_type& context, const vertex_type& vertex,
              const sum_tag_type& msg)
    {
        sum_tag = msg.tag;
    }

    edge_dir_type gather_edges(icontext_type& context,
                               const vertex_type& vertex) const
    {
        return graphlab::NO_EDGES;
    }

    void apply(icontext_type& context, vertex_type& vertex,
               const graphlab::empty& empty)
    {
        changed = false;
        if (vertex.data().tag ^ sum_tag) {
            changed = true;
            vertex.data().tag |= sum_tag;
        }

        if (vertex.data().tag == 3) {
            context.stop();
        }
    }

    edge_dir_type scatter_edges(icontext_type& context,
                                const vertex_type& vertex) const
    {
        if (changed) {
            if (vertex.data().tag == 2)
                return graphlab::IN_EDGES;
            else if (vertex.data().tag == 1)
                return graphlab::OUT_EDGES;
        }

        return graphlab::NO_EDGES;
    }

    void scatter(icontext_type& context, const vertex_type& vertex,
                 edge_type& edge) const
    {

        const vertex_type other = get_other_vertex(edge, vertex);
        tag_type newt = vertex.data().tag;
        const sum_tag_type msg(newt);
        context.signal(other, msg);
    }
};

struct reach_writer {
    std::string save_vertex(const graph_type::vertex_type& vtx)
    {
        std::stringstream strm;
        strm << vtx.id() << "\t" << vtx.data().tag << "\n";
        if (vtx.data().tag != 1)
            return "";
        else
            return strm.str();
    }
    std::string save_edge(graph_type::edge_type e)
    {
        return "";
    }
};

bool line_parser(graph_type& graph, const std::string& filename,
                 const std::string& textline)
{

    std::istringstream ssin(textline);
    graphlab::vertex_id_type vid;
    ssin >> vid;
    int out_nb;
    ssin >> out_nb;

    if (out_nb == 0)
        graph.add_vertex(vid);
    while (out_nb--) {
        graphlab::vertex_id_type other_vid;
        ssin >> other_vid;
        graph.add_edge(vid, other_vid);
    }
    return true;
}
int map_tag(const graph_type::vertex_type& v)
{
    return v.data().tag == 1 ? 1 : 0;
}

int main(int argc, char** argv)
{
    graphlab::mpi_tools::init(argc, argv);

    char* input_file = argv[1];
    char* output_file = argv[2];
    SOURCE = atoi(argv[3]);
    graphlab::distributed_control dc;
    global_logger().set_log_level(LOG_INFO);

    graphlab::timer t;
    t.start();
    graph_type graph(dc);
    graph.load(input_file, line_parser);
    graph.finalize();

    dc.cout() << "Loading graph in " << t.current_time() << " seconds"
              << std::endl;
    std::string exec_type = "synchronous";
    graphlab::omni_engine<reach> engine(dc, graph, exec_type);

    engine.signal(SOURCE, sum_tag_type(1));
    if (DEST != -1)
        engine.signal(DEST, sum_tag_type(2));

    engine.start();

    dc.cout() << "Finished Running engine in " << engine.elapsed_seconds()
              << " seconds." << std::endl;

    const double total_reach = graph.map_reduce_vertices<int>(map_tag);
    dc.cout() << "Total reach: " << total_reach << std::endl;

    t.start();

    graph.save(output_file, reach_writer(), false, // set to true if each output file is to be gzipped
               true, // whether vertices are saved
               false); // whether edges are saved
    dc.cout() << "Dumping graph in " << t.current_time() << " seconds"
              << std::endl;

    graphlab::mpi_tools::finalize();
}
