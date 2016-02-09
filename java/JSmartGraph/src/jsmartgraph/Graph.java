/*
 *  "SmartGraph", the small library for store and manipulations over graphs.
 *  Copyright (C) 2011-2016 by Serg V. Zhdanovskih (aka Alchemist, aka Norseman).
 *
 *  This file is part of "GEDKeeper".
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package jsmartgraph;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class Graph implements IGraph
{
    private static final class DefaultDataProvider implements IDataProvider
    {
        public Vertex createVertexEx()
        {
            return new Vertex();
        }

        @Override
        public IVertex createVertex()
        {
            return (IVertex) this.createVertexEx();
        }

        public Edge createEdgeEx(Vertex u, Vertex v, int cost, Object value)
        {
            return new Edge(u, v, cost, value);
        }

        @Override
        public IEdge createEdge(IVertex u, IVertex v, int cost, Object value)
        {
            return (IEdge) this.createEdgeEx((Vertex) u, (Vertex) v, cost, value);
        }
    }

    private class PathCandidate
    {
        public final IVertex Node;
        public final PathCandidate Next;

        public PathCandidate(IVertex node, PathCandidate next)
        {
            this.Node = node;
            this.Next = next;
        }
    }

    private final IDataProvider fProvider;
    private final ArrayList<IEdge> fEdgesList;
    private final ArrayList<IVertex> fVerticesList;
    private final HashMap<String, IVertex> fVerticesDictionary;

    @Override
    public List<IVertex> getVertices()
    {
        return this.fVerticesList;
    }

    @Override
    public List<IEdge> getEdges()
    {
        return this.fEdgesList;
    }

    public Graph()
    {
        this(new DefaultDataProvider());
    }

    public Graph(IDataProvider provider)
    {
        this.fProvider = provider;
        this.fVerticesList = new ArrayList<>();
        this.fEdgesList = new ArrayList<>();
        this.fVerticesDictionary = new HashMap<>();
    }

    public void clear()
    {
        for (IVertex vertex : this.fVerticesList) {
            vertex.setEdgeIn(null);
            vertex.getEdgesOut().clear();
        }

        this.fEdgesList.clear();
        this.fVerticesList.clear();
        this.fVerticesDictionary.clear();
    }

    public IVertex addVertex(Object data)
    {
        IVertex result = this.fProvider.createVertex();
        result.setValue(data);
        this.fVerticesList.add(result);

        return result;
    }

    public IVertex addVertex(String sign, Object data)
    {
        IVertex result = this.addVertex(data);
        result.setSign(sign);
        this.fVerticesDictionary.put(sign, result);

        return result;
    }

    public boolean addUndirectedEdge(IVertex source, IVertex target, int cost, Object srcValue, Object tgtValue)
    {
        IEdge edge1 = this.addDirectedEdge(source, target, cost, srcValue);
        IEdge edge2 = this.addDirectedEdge(target, source, cost, tgtValue);

        return (edge1 != null && edge2 != null);
    }

    public IEdge addDirectedEdge(String sourceSign, String targetSign, int cost, Object edgeValue)
    {
        IVertex source = this.findVertex(sourceSign);
        IVertex target = this.findVertex(targetSign);

        return this.addDirectedEdge(source, target, cost, edgeValue);
    }

    public IEdge addDirectedEdge(IVertex source, IVertex target, int cost, Object edgeValue)
    {
        if (source == null || target == null || source == target) {
            return null;
        }

        IEdge resultEdge = this.fProvider.createEdge(source, target, cost, edgeValue);
        source.getEdgesOut().add(resultEdge);
        this.fEdgesList.add(resultEdge);

        return resultEdge;
    }

    public void deleteVertex(IVertex vertex)
    {
        if (vertex == null) {
            return;
        }

        for (int i = this.fEdgesList.size() - 1; i >= 0; i--) {
            IEdge edge = this.fEdgesList.get(i);

            if (edge.getSource() == vertex || edge.getTarget() == vertex) {
                this.deleteEdge(edge);
            }
        }

        this.fVerticesList.remove(vertex);
    }

    public void deleteEdge(IEdge edge)
    {
        if (edge == null) {
            return;
        }

        IVertex src = edge.getSource();
        src.getEdgesOut().remove(edge);

        this.fEdgesList.remove(edge);
    }

    public IVertex findVertex(String sign)
    {
        return this.fVerticesDictionary.get(sign);
    }

    public void findPathTree(IVertex root)
    {
        if (root == null) {
            return;
        }

        // reset path tree
        for (IVertex node : this.fVerticesList) {
            node.setDist(Integer.MAX_VALUE);
            node.setVisited(false);
            node.setEdgeIn(null);
        }

        // init root
        root.setDist(0);
        root.setVisited(true);
        root.setEdgeIn(null);

        PathCandidate topCandidate = new PathCandidate(root, null);

        // processing
        while (topCandidate != null) {
            IVertex topNode = topCandidate.Node;
            topCandidate = topCandidate.Next;

            int nodeDist = topNode.getDist();
            topNode.setVisited(false);

            for (IEdge link : topNode.getEdgesOut()) {
                IVertex target = link.getTarget();
                int newDist = nodeDist + link.getCost();

                if (newDist < target.getDist()) {
                    target.setDist(newDist);
                    target.setEdgeIn(link);

                    if (!target.getVisited()) {
                        target.setVisited(true);
                        topCandidate = new PathCandidate(target, topCandidate);
                    }
                }
            }
        }
    }

    public List<IEdge> getPath(IVertex target)
    {
        List<IEdge> result = new ArrayList<>();

        if (target != null) {
            IEdge edge = target.getEdgeIn();
            while (edge != null) {
                result.add(0, edge);
                edge = edge.getSource().getEdgeIn();
            }
        }

        return result;
    }
}
