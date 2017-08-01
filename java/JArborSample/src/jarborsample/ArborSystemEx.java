/*
 *  ArborGVT - a graph vizualization toolkit
 *
 *  Physics code derived from springy.js, copyright (c) 2010 Dennis Hotson
 *  JavaScript library, copyright (c) 2011 Samizdat Drafting Co.
 *
 *  Fork and C# implementation, copyright (c) 2012,2016 by Serg V. Zhdanovskih.
 *  Fork and Java implementation, copyright (c) 2016 by Serg V. Zhdanovskih.
 */
package jarborsample;

import arborgvt.ArborNode;
import arborgvt.ArborSystem;
import arborgvt.IArborRenderer;

/**
 *
 * @author Serg V. Zhdanovskih
 */
public final class ArborSystemEx extends ArborSystem
{
    public ArborSystemEx(double repulsion, double stiffness, double friction, IArborRenderer renderer)
    {
        super(repulsion, stiffness, friction, renderer);
    }

    @Override
    protected ArborNode createNode(String sign)
    {
        return new ArborNodeEx(sign);
    }
}
