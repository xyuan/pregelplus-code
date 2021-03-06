package org.apache.giraph.examples;

import org.apache.giraph.edge.Edge;
import org.apache.giraph.edge.EdgeFactory;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.InputSplit;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.apache.giraph.io.formats.TextVertexInputFormat;
import com.google.common.collect.Lists;

import java.io.IOException;
import java.util.List;
import java.util.regex.Pattern;

/**
 * Input format for HashMin IntWritable, NullWritable, NullWritable Vertex ,
 * Vertex Value, Edge Value Graph vertex \t neighbor1 neighbor 2
 */
public class ReachabilityInputFormat extends
	TextVertexInputFormat<IntWritable, IntWritable, IntWritable> {
    /** Separator of the vertex and neighbors */
    private static final Pattern SEPARATOR = Pattern.compile("[\t ]");

    @Override
    public TextVertexReader createVertexReader(InputSplit split,
	    TaskAttemptContext context) throws IOException {
	return new ReachabiliyVertexReader();
    }

    /**
     * Vertex reader associated with {@link IntIntNullTextInputFormat}.
     */
    public class ReachabiliyVertexReader extends
	    TextVertexReaderFromEachLineProcessed<String[]> {
	private IntWritable id;

	@Override
	protected String[] preprocessLine(Text line) throws IOException {
	    String[] tokens = SEPARATOR.split(line.toString());
	    id = new IntWritable(Integer.parseInt(tokens[0]));
	    return tokens;
	}

	@Override
	protected IntWritable getId(String[] tokens) throws IOException {
	    return id;
	}

	@Override
	protected IntWritable getValue(String[] tokens) throws IOException {
	    if(id.get() == Reachability.SOURCE)
		return new IntWritable(1);
	    else if(id.get() == Reachability.DEST)
		return new IntWritable(2);
	    else
		return  new IntWritable(0);
	}

	@Override
	protected Iterable<Edge<IntWritable, IntWritable>> getEdges(
		String[] tokens) throws IOException {
	    List<Edge<IntWritable, IntWritable>> edges = Lists
		    .newArrayListWithCapacity( (tokens.length - 2) / 2) ;
	    for (int n = 2; n < tokens.length; n+=2) {
		edges.add(EdgeFactory.create(
			new IntWritable(Integer.parseInt(tokens[n])),
			new IntWritable(Integer.parseInt(tokens[n + 1]))));
	    }
	    return edges;
	}
    }
}