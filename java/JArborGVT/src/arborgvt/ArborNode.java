/*
 *  ArborGVT - a graph vizualization toolkit
 *
 *  Physics code derived from springy.js, copyright (c) 2010 Dennis Hotson
 *  JavaScript library, copyright (c) 2011 Samizdat Drafting Co.
 *
 *  Fork and C# implementation, copyright (c) 2012,2016 by Serg V. Zhdanovskih.
 *  Fork and Java implementation, copyright (c) 2016 by Serg V. Zhdanovskih.
 */
package arborgvt;

/**
 *
 * @author Serg V. Zhdanovskih
 */
public class ArborNode
{
    public String Sign;
    public Object Data;

    public boolean Fixed;
    public double Mass;
    public ArborPoint Pt;

    public ArborPoint V;
    public ArborPoint F;

    public ArborNode(String sign)
    {
        this.Sign = sign;

        this.Fixed = false;
        this.Mass = 1;
        this.Pt = ArborPoint.Null;

        this.V = new ArborPoint(0, 0);
        this.F = new ArborPoint(0, 0);
    }

    public void applyForce(ArborPoint a)
    {
        this.F = this.F.add(a.div(this.Mass));
    }
}
