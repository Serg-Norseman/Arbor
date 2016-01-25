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

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

/**
 *
 * @author Serg V. Zhdanovskih
 */
public final class BarnesHutTree
{
    private static final class Branch
    {
        public ArborPoint Origin = ArborPoint.Null;
        public ArborPoint Size = ArborPoint.Null;
        public Object[] Q = new Object[]{null, null, null, null};
        public double Mass = 0.0;
        public ArborPoint Pt = ArborPoint.Null;
    }

    private static final int QNe = 0;
    private static final int QNw = 1;
    private static final int QSe = 2;
    private static final int QSw = 3;
    private static final int QNone = 4;

    private final double fDist; // default = 0.5
    private final Branch fRoot;

    public BarnesHutTree(ArborPoint origin, ArborPoint h, double dist)
    {
        this.fDist = dist;
        this.fRoot = new Branch();
        this.fRoot.Origin = origin;
        this.fRoot.Size = h.sub(origin);
    }

    private static int getQuad(ArborNode i, Branch f)
    {
        try {
            if (i.Pt.exploded()) {
                return QNone;
            }
            ArborPoint h = i.Pt.sub(f.Origin);
            ArborPoint g = f.Size.div(2);
            if (h.Y < g.Y) {
                return (h.X < g.X) ? QNw : QNe;
            } else {
                return (h.X < g.X) ? QSw : QSe;
            }
        } catch (Exception ex) {
            System.out.println("BarnesHutTree.getQuad(): " + ex.getMessage());
            return QNone;
        }
    }

    public void insert(ArborNode j)
    {
        try {
            Branch f = fRoot;
            List<ArborNode> gst = new ArrayList<>();
            gst.add(j);
            while (gst.size() > 0) {
                ArborNode h = gst.get(0);
                gst.remove(0);

                double m = h.Mass;
                int p = getQuad(h, f);
                Object fp = f.Q[p];

                if (fp == null) {
                    f.Q[p] = h;

                    f.Mass += m;
                    if (!f.Pt.isNull()) {
                        f.Pt = f.Pt.add(h.Pt.mul(m));
                    } else {
                        f.Pt = h.Pt.mul(m);
                    }
                } else if (fp instanceof Branch) {
                    f.Mass += (m);
                    if (!f.Pt.isNull()) {
                        f.Pt = f.Pt.add(h.Pt.mul(m));
                    } else {
                        f.Pt = h.Pt.mul(m);
                    }

                    f = (Branch) fp;

                    gst.add(0, h);
                } else {
                    ArborPoint l = f.Size.div(2);
                    ArborPoint n = new ArborPoint(f.Origin.X, f.Origin.Y);

                    if (p == QSe || p == QSw) {
                        n.Y += l.Y;
                    }
                    if (p == QNe || p == QSe) {
                        n.X += l.X;
                    }

                    ArborNode o = (ArborNode) fp;
                    fp = new Branch();
                    ((Branch) fp).Origin = n;
                    ((Branch) fp).Size = l;
                    f.Q[p] = fp;

                    f.Mass = m;
                    f.Pt = h.Pt.mul(m);

                    f = (Branch) fp;

                    if (o.Pt.X == h.Pt.X && o.Pt.Y == h.Pt.Y) {
                        double k = l.X * 0.08;
                        double i = l.Y * 0.08;
                        o.Pt.X = Math.min(n.X + l.X, Math.max(n.X, o.Pt.X - k / 2 + ArborSystem.NextRndDouble() * k));
                        o.Pt.Y = Math.min(n.Y + l.Y, Math.max(n.Y, o.Pt.Y - i / 2 + ArborSystem.NextRndDouble() * i));
                    }

                    gst.add(o);
                    gst.add(0, h);
                }
            }
        } catch (Exception ex) {
            System.out.println("BarnesHutTree.insert(): " + ex.getMessage());
        }
    }

    public void applyForces(ArborNode m, double g)
    {
        try {
            Queue<Object> f = new LinkedList<>();

            f.add(fRoot);
            while (f.size() > 0) {
                Object obj = f.poll();
                if (obj == null || obj == m) {
                    continue;
                }

                ArborPoint ptx, i, k;
                double l, kMag, massx;

                if (obj instanceof ArborNode) {
                    ArborNode node = (ArborNode) obj;
                    massx = node.Mass;
                    ptx = node.Pt;

                    k = m.Pt.sub(ptx);
                    kMag = k.magnitude();

                    l = Math.max(1, kMag);
                    i = ((kMag > 0) ? k : ArborPoint.newRnd(1)).normalize();
                    m.applyForce(i.mul(g * massx).div(l * l));
                } else {
                    Branch branch = (Branch) obj;
                    massx = branch.Mass;
                    ptx = branch.Pt.div(massx);

                    k = m.Pt.sub(ptx);
                    kMag = k.magnitude();

                    double h = Math.sqrt(branch.Size.X * branch.Size.Y);
                    if (h / kMag > fDist) {
                        f.add(branch.Q[QNe]);
                        f.add(branch.Q[QNw]);
                        f.add(branch.Q[QSe]);
                        f.add(branch.Q[QSw]);
                    } else {
                        l = Math.max(1, kMag);
                        i = ((kMag > 0) ? k : ArborPoint.newRnd(1)).normalize();
                        m.applyForce(i.mul(g * massx).div(l * l));
                    }
                }
            }
        } catch (Exception ex) {
            System.out.println("BarnesHutTree.applyForces(): " + ex.getMessage());
        }
    }
}
