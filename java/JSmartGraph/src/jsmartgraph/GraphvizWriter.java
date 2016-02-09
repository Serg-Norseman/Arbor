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

public final class GraphvizWriter
{
    private final StringBuilder fBuffer;

    public GraphvizWriter(String name)
    {
        fBuffer = new StringBuilder();
        fBuffer.append("digraph " + name.trim().replace(' ', '_') + "{");
    }

    public GraphvizWriter(String name, String[] options)
    {
        this(name);
        fBuffer.append("digraph " + name.trim().replace(' ', '_') + "{");
        for (String option : options) {
            fBuffer.append("\t" + option + ";");
        }
    }

    public void writeEdge(String from, String to)
    {
        fBuffer.append(String.format("\"{0}\" -> \"{1}\";", from, to));
    }

    public void writeNode(String id, String name, String style, String color, String shape)
    {
        fBuffer.append(String.format("\"{0}\" [ label=\"{1}\",shape=\"{2}\",style=\"{3}\",color=\"{4}\" ];", id, name, shape, style, color));
    }

    public void saveFile(String path)
    {
        fBuffer.append("}");
        /*using (StreamWriter SW = new StreamWriter(path, false, Encoding.GetEncoding(1251)))
			{
				SW.Write(fBuffer.toString());
				System.Console.Write(fBuffer.toString());
			}*/
    }
}
