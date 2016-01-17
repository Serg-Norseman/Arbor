/*
 *  ArborGVT - a graph vizualization toolkit
 *
 *  Physics code derived from springy.js, copyright (c) 2010 Dennis Hotson
 *  JavaScript library, copyright (c) 2011 Samizdat Drafting Co.
 *
 *  Fork and C# implementation, copyright (c) 2012,2016 by Serg V. Zhdanovskih.
 */

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace ArborGVT
{
    internal class Branch
    {
        public ArborPoint Origin = ArborPoint.Null;
        public ArborPoint Size = ArborPoint.Null;
        public object[] Q = new object[4] { null, null, null, null };
        public double Mass = 0.0;
        public ArborPoint Pt = ArborPoint.Null;
    }

    internal class BarnesHutTree
    {
        private const int QNe = 0;
        private const int QNw = 1;
        private const int QSe = 2;
        private const int QSw = 3;
        private const int QNone = 4;

        private readonly double fDist; // default = 0.5
        private readonly Branch fRoot;

        public BarnesHutTree(ArborPoint origin, ArborPoint h, double dist)
        {
            this.fDist = dist;
            this.fRoot = new Branch();
            this.fRoot.Origin = origin;
            this.fRoot.Size = h.sub(origin);
        }

        private static int getQuad(ArborNode i, Branch f)
        {
            try
            {
                if (i.Pt.exploded())
                {
                    return QNone;
                }
                ArborPoint h = i.Pt.sub(f.Origin);
                ArborPoint g = f.Size.div(2);
                if (h.Y < g.Y)
                {
                    return (h.X < g.X) ? QNw : QNe;
                }
                else
                {
                    return (h.X < g.X) ? QSw : QSe;
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine("BarnesHutTree.getQuad(): " + ex.Message);
                return QNone;
            }
        }

        public void insert(ArborNode j)
        {
            try
            {
                Branch f = fRoot;
                List<ArborNode> gst = new List<ArborNode>();
                gst.Add(j);
                while (gst.Count > 0)
                {
                    ArborNode h = gst[0];
                    gst.RemoveAt(0);

                    double m = h.Mass;
                    int p = getQuad(h, f);
                    object fp = f.Q[p];

                    if (fp == null)
                    {
                        f.Q[p] = h;

                        f.Mass += m;
                        if (!f.Pt.isNull())
                        {
                            f.Pt = f.Pt.add(h.Pt.mul(m));
                        }
                        else
                        {
                            f.Pt = h.Pt.mul(m);
                        }
                    }
                    else
                    {
                        if (fp is Branch)
                        {
                            f.Mass += (m);
                            if (!f.Pt.isNull())
                            {
                                f.Pt = f.Pt.add(h.Pt.mul(m));
                            }
                            else
                            {
                                f.Pt = h.Pt.mul(m);
                            }

                            f = fp as Branch;

                            gst.Insert(0, h);
                        }
                        else
                        {
                            ArborPoint l = f.Size.div(2);
                            ArborPoint n = new ArborPoint(f.Origin.X, f.Origin.Y);

                            if (p == QSe || p == QSw)
                            {
                                n.Y += l.Y;
                            }
                            if (p == QNe || p == QSe)
                            {
                                n.X += l.X;
                            }

                            ArborNode o = fp as ArborNode;
                            fp = new Branch();
                            (fp as Branch).Origin = n;
                            (fp as Branch).Size = l;
                            f.Q[p] = fp;

                            f.Mass = m;
                            f.Pt = h.Pt.mul(m);

                            f = fp as Branch;

                            if (o.Pt.X == h.Pt.X && o.Pt.Y == h.Pt.Y)
                            {
                                double k = l.X * 0.08;
                                double i = l.Y * 0.08;
                                o.Pt.X = Math.Min(n.X + l.X, Math.Max(n.X, o.Pt.X - k / 2 + ArborSystem.NextRndDouble() * k));
                                o.Pt.Y = Math.Min(n.Y + l.Y, Math.Max(n.Y, o.Pt.Y - i / 2 + ArborSystem.NextRndDouble() * i));
                            }

                            gst.Add(o);
                            gst.Insert(0, h);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine("BarnesHutTree.insert(): " + ex.Message);
            }
        }

        public void applyForces(ArborNode m, double g)
        {
            try
            {
                Queue<object> f = new Queue<object>();

                f.Enqueue(fRoot);
                while (f.Count > 0)
                {
                    object obj = f.Dequeue();
                    if (obj == null || obj == m) continue;

                    ArborPoint ptx, i, k;
                    double l, kMag, massx;

                    if (obj is ArborNode)
                    {
                        ArborNode node = (obj as ArborNode);
                        massx = node.Mass;
                        ptx = node.Pt;

                        k = m.Pt.sub(ptx);
                        kMag = k.magnitude();

                        l = Math.Max(1, kMag);
                        i = ((kMag > 0) ? k : ArborPoint.newRnd(1)).normalize();
                        m.applyForce(i.mul(g * massx).div(l * l));
                    }
                    else
                    {
                        Branch branch = (obj as Branch);
                        massx = branch.Mass;
                        ptx = branch.Pt.div(massx);

                        k = m.Pt.sub(ptx);
                        kMag = k.magnitude();

                        double h = Math.Sqrt(branch.Size.X * branch.Size.Y);
                        if (h / kMag > fDist)
                        {
                            f.Enqueue(branch.Q[QNe]);
                            f.Enqueue(branch.Q[QNw]);
                            f.Enqueue(branch.Q[QSe]);
                            f.Enqueue(branch.Q[QSw]);
                        }
                        else
                        {
                            l = Math.Max(1, kMag);
                            i = ((kMag > 0) ? k : ArborPoint.newRnd(1)).normalize();
                            m.applyForce(i.mul(g * massx).div(l * l));
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine("BarnesHutTree.applyForces(): " + ex.Message);
            }
        }
    }
}
