//
//  Arbor - version 0.91
//  a graph vizualization toolkit
//
//  Copyright (c) 2011 Samizdat Drafting Co.
//  Physics code derived from springy.js, copyright (c) 2010 Dennis Hotson
// 

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Timers;

namespace ArborGVT
{
    internal class PSBounds
    {
        public ArborPoint LeftTop = ArborPoint.Null;
        public ArborPoint RightBottom = ArborPoint.Null;
        
        public PSBounds(ArborPoint leftTop, ArborPoint rightBottom)
        {
            this.LeftTop = leftTop;
            this.RightBottom = rightBottom;
        }
    }

    public sealed class ArborSystem : IDisposable
    {
        private static readonly Random _random = new Random();

        private readonly int[] margins = new int[4] { 20, 20, 20, 20 };
        private const double Mag = 0.04;
        private const double Theta = 0.4;

        private bool fAutoStop;
        private bool fBusy;
        private bool fDisposed;
        private readonly List<ArborEdge> fEdges;
        private readonly Hashtable fNames;
        private readonly List<ArborNode> fNodes;
        private EventHandler fOnStart;
        private EventHandler fOnStop;
        private readonly IArborRenderer fRenderer;
        private DateTime fPrevTime;
        private double fStopThreshold;
        private Timer fTimer;

        private Size usz;
        private PSBounds n_bnd = null;
        private PSBounds o_bnd = null;

        public double EnergySum = 0;
        public double EnergyMax = 0;
        public double EnergyMean = 0;

        public double ParamRepulsion = 1000; // отражение, отвращение, отталкивание
        public double ParamStiffness = 600; // церемонность, тугоподвижность
        public double ParamFriction = 0.5; // трение
        public double ParamDt = 0.01; // 0.02;
        public bool ParamGravity = false;
        public double ParamPrecision = 0.6;
        public double ParamTimeout = 1000 / 100;

        #region Properties

        public bool AutoStop
        {
            get { return this.fAutoStop; }
            set { this.fAutoStop = value; }
        }

        public List<ArborNode> Nodes
        {
            get { return this.fNodes; }
        }

        public List<ArborEdge> Edges
        {
            get { return this.fEdges; }
        }

        public event EventHandler OnStart
        {
            add {
                this.fOnStart = value;
            }
            remove {
                if (this.fOnStart == value) {
                    this.fOnStart = null;
                }
            }
        }

        public event EventHandler OnStop
        {
            add {
                this.fOnStop = value;
            }
            remove {
                if (this.fOnStop == value) {
                    this.fOnStop = null;
                }
            }
        }

        public double StopThreshold
        {
            get { return this.fStopThreshold; }
            set { this.fStopThreshold = value; }
        }

        #endregion

        public ArborSystem(double repulsion, double stiffness, double friction, IArborRenderer renderer)
        {
            this.fAutoStop = true;
            this.fBusy = false;
            this.fNames = new Hashtable();
            this.fNodes = new List<ArborNode>();
            this.fEdges = new List<ArborEdge>();
            this.fRenderer = renderer;
            this.fPrevTime = DateTime.FromBinary(0);
            this.fStopThreshold = /*0.05*/ 0.7;
            this.fTimer = null;

            this.ParamRepulsion = repulsion;
            this.ParamStiffness = stiffness;
            this.ParamFriction = friction;
        }

        public void Dispose()
        {
            if (!this.fDisposed)
            {
                this.stop();
                this.fDisposed = true;
            }
        }

        public void start()
        {
            if (fOnStart != null) fOnStart(this, new EventArgs());

            if (fTimer != null) {
                return;
            }
            fPrevTime = DateTime.FromBinary(0);

            fTimer = new System.Timers.Timer();
            fTimer.AutoReset = true;
            fTimer.Interval = ParamTimeout;
            fTimer.Elapsed += this.tickTimer;
            fTimer.Start();
        }

        public void stop()
        {
            if (fTimer != null) {
                fTimer.Stop();
                fTimer.Dispose();
                fTimer = null;
            }

            if (fOnStop != null) fOnStop(this, new EventArgs());
        }

        public ArborNode addNode(string sign, double x, double y)
        {
            ArborNode node = this.getNode(sign);

            if (node != null) {
                return node;
            } else {
                node = new ArborNode(sign);
                node.Pt = new ArborPoint(x, y);
                
                fNames.Add(sign, node);
                fNodes.Add(node);

                return node;
            }
        }

        public ArborNode addNode(string sign)
        {
            double xx = o_bnd.LeftTop.X + (o_bnd.RightBottom.X - o_bnd.LeftTop.X) * ArborSystem.NextRndDouble();
            double yy = o_bnd.LeftTop.Y + (o_bnd.RightBottom.Y - o_bnd.LeftTop.Y) * ArborSystem.NextRndDouble();

            return this.addNode(sign, xx, yy);
        }

        public ArborNode getNode(string sign)
        {
            return (ArborNode)fNames[sign];
        }

        public ArborEdge addEdge(string srcSign, string tgtSign, int len = 1)
        {
            ArborNode src = this.getNode(srcSign);
            src = (src != null) ? src : this.addNode(srcSign);

            ArborNode tgt = this.getNode(tgtSign);
            tgt = (tgt != null) ? tgt : this.addNode(tgtSign);

            ArborEdge x = null;
            if (src != null && tgt != null) {
                foreach (ArborEdge edge in fEdges) {
                    if (edge.Source == src && edge.Target == tgt) {
                        x = edge;
                        break;
                    }
                }
            }

            if (x == null) {
                x = new ArborEdge(src, tgt, len, ParamStiffness);
                fEdges.Add(x);
            }

            return x;
        }

        public void setScreenSize(int width, int height)
        {
            usz.Width = width;
            usz.Height = height;
            this.updateBounds();
        }

        public ArborPoint toScreen(ArborPoint pt)
        {
            if (n_bnd == null) return ArborPoint.Null;

            ArborPoint v = n_bnd.RightBottom.sub(n_bnd.LeftTop);
            double sx = margins[3] + pt.sub(n_bnd.LeftTop).div(v.X).X * (usz.Width - (margins[1] + margins[3]));
            double sy = margins[0] + pt.sub(n_bnd.LeftTop).div(v.Y).Y * (usz.Height - (margins[0] + margins[2]));
            return new ArborPoint(sx, sy);
        }

        public ArborPoint fromScreen(double sx, double sy)
		{
			if (n_bnd == null) return ArborPoint.Null;

			ArborPoint x = n_bnd.RightBottom.sub(n_bnd.LeftTop);
			double w = (sx - margins[3]) / (usz.Width - (margins[1] + margins[3])) * x.X + n_bnd.LeftTop.X;
			double v = (sy - margins[0]) / (usz.Height - (margins[0] + margins[2])) * x.Y + n_bnd.LeftTop.Y;
			return new ArborPoint(w, v);
		}

        public ArborNode nearest(int sx, int sy)
        {
            ArborPoint x = this.fromScreen(sx, sy);
            
            ArborNode resNode = null;
            double minDist = +1.0;

            foreach (ArborNode node in this.fNodes)
            {
                ArborPoint z = node.Pt;
                if (z.exploded()) {
                    continue;
                }

                double dist = z.sub(x).magnitude();
                if (dist < minDist) {
                    resNode = node;
                    minDist = dist;
                }
            }

            //minDist = this.toScreen(resNode.Pt).sub(this.toScreen(x)).magnitude();
            return resNode;
        }

        private PSBounds getActualBounds()
        {
            ArborPoint tl = new ArborPoint(-1, -1);
            ArborPoint br = new ArborPoint(1, 1);

            foreach (ArborNode node in this.fNodes)
            {
                ArborPoint pt = node.Pt;
                if (pt.exploded()) continue;

                if (pt.X < tl.X) tl.X = pt.X;
                if (pt.Y < tl.Y) tl.Y = pt.Y;
                if (pt.X > br.X) br.X = pt.X;
                if (pt.Y > br.Y) br.Y = pt.Y;
            }

            tl.X -= 1.2;
            tl.Y -= 1.2;
            br.X += 1.2;
            br.Y += 1.2;
            return new PSBounds(tl, br);
        }

        private void updateBounds()
        {
            try
            {
                o_bnd = this.getActualBounds();

                ArborPoint sz = o_bnd.RightBottom.sub(o_bnd.LeftTop);
                ArborPoint cent = o_bnd.LeftTop.add(sz.div(2));

                const double x = 4.0;
                ArborPoint d = new ArborPoint(Math.Max(sz.X, x), Math.Max(sz.Y, x)).div(2);
                o_bnd.LeftTop = cent.sub(d);
                o_bnd.RightBottom = cent.add(d);

                if (n_bnd == null) {
                    n_bnd = o_bnd;
                    return;
                }

                ArborPoint nbRB = n_bnd.RightBottom.add(o_bnd.RightBottom.sub(n_bnd.RightBottom).mul(Mag));
                ArborPoint nbLT = n_bnd.LeftTop.add(o_bnd.LeftTop.sub(n_bnd.LeftTop).mul(Mag));

                ArborPoint a = new ArborPoint(n_bnd.LeftTop.sub(nbLT).magnitude(), n_bnd.RightBottom.sub(nbRB).magnitude());

                if (a.X * usz.Width > 1 || a.Y * usz.Height > 1) {
                    n_bnd = new PSBounds(nbLT, nbRB);
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine("ArborSystem.updateBounds(): " + ex.Message);
            }
        }

        private void tickTimer(object sender, ElapsedEventArgs e)
        {
            if (this.fBusy) return;
            this.fBusy = true;
            try
            {
                this.updatePhysics();
                this.updateBounds();

                if (fRenderer != null) {
                    fRenderer.Invalidate();
                }

                if (this.fAutoStop) {
                    if (EnergyMean <= this.fStopThreshold) {
                        if (fPrevTime == DateTime.FromBinary(0)) {
                            fPrevTime = DateTime.Now;
                        }
                        TimeSpan ts = DateTime.Now - fPrevTime;
                        if (ts.TotalMilliseconds > 1000) {
                            this.stop();
                        }
                    } else {
                        fPrevTime = DateTime.FromBinary(0);
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine("ArborSystem.tickTimer(): " + ex.Message);
            }
            this.fBusy = false;
        }

        private void updatePhysics()
        {
            try
            {
                // tend particles
                foreach (ArborNode p in fNodes) {
                    p.V.X = 0;
                    p.V.Y = 0;
                }

                // euler integrator
                if (ParamRepulsion > 0) {
                    if (Theta > 0) {
                        this.applyBarnesHutRepulsion();
                    } else {
                        this.applyBruteForceRepulsion();
                    }
                }

                if (ParamStiffness > 0) {
                    this.applySprings();
                }

                this.updateVelocityAndPosition(ParamDt);
            }
            catch (Exception ex)
            {
                Debug.WriteLine("ArborSystem.updatePhysics(): " + ex.Message);
            }
        }

        private void applyBruteForceRepulsion()
        {
            foreach (ArborNode p in fNodes) {
                foreach (ArborNode r in fNodes) {
                    if (p != r) {
                        ArborPoint u = p.Pt.sub(r.Pt);
                        double v = Math.Max(1, u.magnitude());
                        ArborPoint t = ((u.magnitude() > 0) ? u : ArborPoint.newRnd(1)).normalize();
                        p.applyForce(t.mul(ParamRepulsion * r.Mass * 0.5).div(v * v * 0.5));
                        r.applyForce(t.mul(ParamRepulsion * p.Mass * 0.5).div(v * v * -0.5));
                    }
                }
            }
        }

        private void applyBarnesHutRepulsion()
        {
            BarnesHutTree bht = new BarnesHutTree(o_bnd.LeftTop, o_bnd.RightBottom, Theta);

            foreach (ArborNode node in fNodes) {
                bht.insert(node);
            }

            foreach (ArborNode node in fNodes) {
                bht.applyForces(node, ParamRepulsion);
            }
        }

        private void applySprings()
        {
            foreach (ArborEdge edge in fEdges) {
                ArborPoint s = edge.Target.Pt.sub(edge.Source.Pt);

                double q = edge.Length - s.magnitude();
                ArborPoint r = ((s.magnitude() > 0) ? s : ArborPoint.newRnd(1)).normalize();

                edge.Source.applyForce(r.mul(edge.Stiffness * q * -0.5));
                edge.Target.applyForce(r.mul(edge.Stiffness * q * 0.5));
            }
        }

        private void updateVelocityAndPosition(double dt)
        {
            double eSum = 0;
            double eMax = 0;

            // calc center drift
            int size = fNodes.Count;
            bool hasCenterDrift = (size != 0);
            ArborPoint drift = new ArborPoint(0, 0);
            if (hasCenterDrift) {
                ArborPoint r = new ArborPoint(0, 0);
                foreach (ArborNode node in fNodes) {
                    r = r.add(node.Pt);
                }
                drift = r.div(-size);
            }

            foreach (ArborNode node in fNodes) {
                // apply center drift
                if (hasCenterDrift) {
                    node.applyForce(drift);
                }

                // apply center gravity
                if (ParamGravity) {
                    ArborPoint q = node.Pt.mul(-1);
                    node.applyForce(q.mul(ParamRepulsion / 100));
                }

                // update velocities
                if (node.Fixed) {
                    node.V = new ArborPoint(0, 0);
                    node.F = new ArborPoint(0, 0);
                } else {
                    node.V = node.V.add(node.F.mul(dt));
                    node.V = node.V.mul(1 - ParamFriction);

                    node.F.X = node.F.Y = 0;
                    double r = node.V.magnitude();
                    if (r > 1000) {
                        node.V = node.V.div(r * r);
                    }
                }

                // update positions
                node.Pt = node.Pt.add(node.V.mul(dt));

                // update energy
                double x = node.V.magnitude();
                double z = x * x;
                eSum += z;
                eMax = Math.Max(z, eMax);
            }

            EnergySum = eSum;
            EnergyMax = eMax;
            EnergyMean = (size > 0) ? eSum / size : 0;
        }

        internal static double NextRndDouble()
        {
            return _random.NextDouble();
        }
    }
}
