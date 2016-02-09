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
import java.util.List;

public class Vertex implements IVertex
{
    //private static int nextNodeIdx = 1;
    //public int Idx;

    private String fSign;
    private Object fValue;
    private int fDist;
    private boolean fVisited;
    private IEdge fEdgeIn;
    private final List<IEdge> fEdgesOut;

    @Override
    public String getSign()
    {
        return this.fSign;
    }

    @Override
    public void setSign(String value)
    {
        this.fSign = value;
    }

    @Override
    public Object getValue()
    {
        return this.fValue;
    }

    @Override
    public void setValue(Object value)
    {
        this.fValue = value;
    }

    @Override
    public int getDist()
    {
        return this.fDist;
    }

    @Override
    public void setDist(int value)
    {
        this.fDist = value;
    }

    @Override
    public boolean getVisited()
    {
        return this.fVisited;
    }

    @Override
    public void setVisited(boolean value)
    {
        this.fVisited = value;
    }

    @Override
    public IEdge getEdgeIn()
    {
        return this.fEdgeIn;
    }

    @Override
    public void setEdgeIn(IEdge value)
    {
        this.fEdgeIn = value;
    }

    @Override
    public List<IEdge> getEdgesOut()
    {
        return this.fEdgesOut;
    }

    public Vertex()
    {
        //this.Idx = nextNodeIdx++;

        this.fEdgesOut = new ArrayList<>();
    }

    public int CompareTo(Object obj)
    {
        /*if (!(obj is Vertex))
				throw new ArgumentException("Cannot compare two objects");

			return GetHashCode().CompareTo(obj.GetHashCode());*/
        return 0;
    }
}
