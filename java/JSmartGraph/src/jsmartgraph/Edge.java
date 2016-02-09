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

public class Edge implements IEdge
{
    private final int fCost;
    private final Vertex fSource;
    private final Vertex fTarget;
    private final Object fValue;

    @Override
    public int getCost()
    {
        return this.fCost;
    }

    @Override
    public Object getValue()
    {
        return this.fValue;
    }

    @Override
    public IVertex getSource()
    {
        return (IVertex) this.fSource;
    }

    @Override
    public IVertex getTarget()
    {
        return (IVertex) this.fTarget;
    }

    public Edge(Vertex source, Vertex target, int cost, Object value)
    {
        //if (source == null)
        //	throw new ArgumentNullException("source");
        //if (target == null)
        //	throw new ArgumentNullException("target");

        this.fSource = source;
        this.fTarget = target;
        this.fCost = cost;
        this.fValue = value;
    }

    public int CompareTo(Object obj)
    {
        /*if (!(obj is Edge))
				throw new ArgumentException("Cannot compare two objects");

			return GetHashCode().CompareTo(obj.GetHashCode());*/
        return 0;
    }
}
