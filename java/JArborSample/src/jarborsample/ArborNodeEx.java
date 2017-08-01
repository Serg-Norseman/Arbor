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
import java.awt.Color;
import java.awt.geom.Rectangle2D;

/**
 *
 * @author Serg V. Zhdanovskih
 */
public final class ArborNodeEx extends ArborNode
{
    public Color Color;
    public Rectangle2D.Float Box;

    public ArborNodeEx(String sign)
    {
        super(sign);
        this.Color = Color.gray;
    }
}
