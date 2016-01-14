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
    public class PSBounds
    {
        public ArborPoint topleft = ArborPoint.Null;
        public ArborPoint bottomright = ArborPoint.Null;
        
        public PSBounds(ArborPoint topleft, ArborPoint bottomright)
        {
            this.topleft = topleft;
            this.bottomright = bottomright;
        }
    }

    public class ArborSystem : IDisposable
    {
        private static readonly Random _random = new Random();

        private bool fAutoStop;
        private bool fBusy;
        private bool fDisposed;
        private List<ArborEdge> fEdges;
        private Hashtable fNames;
        private List<ArborNode> fNodes;
        private EventHandler fOnStart;
        private EventHandler fOnStop;
        private IArborRenderer fRenderer;
        private DateTime fPrevTime;
        private double fStopThreshold;
        private Timer fTimer;

        private Size usz;
        private double mag = 0.04;
        private int[] margins = new int[4] {20, 20, 20, 20};
        private PSBounds n_bnd = null;
        private PSBounds o_bnd = null;
        private double theta = 0.4;

        public double energy_sum = 0;
        public double energy_max = 0;
        public double energy_mean = 0;

        public double param_repulsion = 1000; // отражение, отвращение
        public double param_stiffness = 600; // церемонность :)
        public double param_friction = 0.5; // трение
        public double param_dt = 0.01; // 0.02;
        public bool param_gravity = false;
        public double param_precision = 0.6;
        public double param_timeout = 1000 / 100;

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

        // repulsion - отталкивание, stiffness - тугоподвижность, friction - сила трения
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

            this.param_repulsion = repulsion;
            this.param_stiffness = stiffness;
            this.param_friction = friction;
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
            fTimer.Interval = param_timeout;
            fTimer.Elapsed += new System.Timers.ElapsedEventHandler(this.tickTimer);
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
            double xx = o_bnd.topleft.x + (o_bnd.bottomright.x - o_bnd.topleft.x) * ArborSystem.NextRndDouble();
            double yy = o_bnd.topleft.y + (o_bnd.bottomright.y - o_bnd.topleft.y) * ArborSystem.NextRndDouble();

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
                x = new ArborEdge(src, tgt, len, param_stiffness);
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

            ArborPoint v = n_bnd.bottomright.sub(n_bnd.topleft);
            double sx = margins[3] + pt.sub(n_bnd.topleft).div(v.x).x * (usz.Width - (margins[1] + margins[3]));
            double sy = margins[0] + pt.sub(n_bnd.topleft).div(v.y).y * (usz.Height - (margins[0] + margins[2]));
            return new ArborPoint(sx, sy);
        }

        public ArborPoint fromScreen(double sx, double sy)
		{
			if (n_bnd == null) return ArborPoint.Null;

			ArborPoint x = n_bnd.bottomright.sub(n_bnd.topleft);
			double w = (sx - margins[3]) / (usz.Width - (margins[1] + margins[3])) * x.x + n_bnd.topleft.x;
			double v = (sy - margins[0]) / (usz.Height - (margins[0] + margins[2])) * x.y + n_bnd.topleft.y;
			return new ArborPoint(w, v);
		}

        public ArborNode nearest(int sx, int sy)
        {
            ArborPoint x = this.fromScreen(sx, sy);
            
            ArborNode w_node = null;
            ArborPoint w_point = ArborPoint.Null;
            double w_distance = +1.0;

            foreach (ArborNode y in this.fNodes)
            {
                ArborPoint z = y.Pt;
                if (z.exploded()) {
                    continue;
                }

                double A = z.sub(x).magnitude();
                if (A < w_distance) {
                    w_node = y;
                    w_point = z;
                    w_distance = A;
                }
            }

            if (w_node != null) {
                w_distance = this.toScreen(w_node.Pt).sub(this.toScreen(x)).magnitude();
                return w_node;
            } else {
                return null;
            }
        }

        private PSBounds getActualBounds()
        {
            ArborPoint tl = new ArborPoint(-1, -1);
            ArborPoint br = new ArborPoint(1, 1);

            foreach (ArborNode node in this.fNodes)
            {
                ArborPoint pt = node.Pt;
                if (pt.exploded()) continue;

                if (pt.x < tl.x) tl.x = pt.x;
                if (pt.y < tl.y) tl.y = pt.y;
                if (pt.x > br.x) br.x = pt.x;
                if (pt.y > br.y) br.y = pt.y;
            }

            tl.x -= 1.2;
            tl.y -= 1.2;
            br.x += 1.2;
            br.y += 1.2;
            return new PSBounds(tl, br);
        }

        private void updateBounds()
        {
            try
            {
                if (usz == null) return;

                o_bnd = this.getActualBounds();

                ArborPoint sz = o_bnd.bottomright.sub(o_bnd.topleft);
                ArborPoint cent = o_bnd.topleft.add(sz.div(2));

                double x = 4.0;
                ArborPoint D = new ArborPoint(Math.Max(sz.x, x), Math.Max(sz.y, x)).div(2);
                o_bnd.topleft = cent.sub(D);
                o_bnd.bottomright = cent.add(D);

                if (n_bnd == null) {
                    n_bnd = o_bnd;
                    return;
                }

                ArborPoint _nb_BR = n_bnd.bottomright.add(o_bnd.bottomright.sub(n_bnd.bottomright).mul(mag));
                ArborPoint _nb_TL = n_bnd.topleft.add(o_bnd.topleft.sub(n_bnd.topleft).mul(mag));

                ArborPoint A = new ArborPoint(n_bnd.topleft.sub(_nb_TL).magnitude(), n_bnd.bottomright.sub(_nb_BR).magnitude());

                if (A.x * usz.Width > 1 || A.y * usz.Height > 1) {
                    n_bnd = new PSBounds(_nb_TL, _nb_BR);
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine("ArborSystem.updateBounds(): " + ex.Message);
            }
        }

        private void tickTimer(object sender, System.Timers.ElapsedEventArgs e)
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
                    if (energy_mean <= this.fStopThreshold) {
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
                    p.v.x = 0;
                    p.v.y = 0;
                }

                // euler integrator
                if (param_repulsion > 0) {
                    if (theta > 0) {
                        this.applyBarnesHutRepulsion();
                    } else {
                        this.applyBruteForceRepulsion();
                    }
                }

                if (param_stiffness > 0) {
                    this.applySprings();
                }

                this.applyCenterDrift();

                if (param_gravity) {
                    this.applyCenterGravity();
                }

                this.updateVelocityAndPosition(param_dt);
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
                        p.applyForce(t.mul(param_repulsion * r.Mass * 0.5).div(v * v * 0.5));
                        r.applyForce(t.mul(param_repulsion * p.Mass * 0.5).div(v * v * -0.5));
                    }
                }
            }
        }

        private void applyBarnesHutRepulsion()
        {
            BarnesHutTree bht = new BarnesHutTree(o_bnd.topleft, o_bnd.bottomright, theta);

            foreach (ArborNode node in fNodes) {
                bht.insert(node);
            }

            foreach (ArborNode node in fNodes) {
                bht.applyForces(node, param_repulsion);
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

        private void applyCenterDrift()
        {
            int size = fNodes.Count;
            if (size == 0) return;

            ArborPoint r = new ArborPoint(0, 0);
            foreach (ArborNode node in fNodes) {
                r = r.add(node.Pt);
            }

            ArborPoint p = r.div(-size);
            foreach (ArborNode node in fNodes) {
                node.applyForce(p);
            }
        }

        private void applyCenterGravity()
        {
            foreach (ArborNode node in fNodes) {
                ArborPoint q = node.Pt.mul(-1);
                node.applyForce(q.mul(param_repulsion / 100));
            }
        }

        private void updateVelocityAndPosition(double dt)
        {
            double eSum = 0;
            double eMax = 0;

            foreach (ArborNode node in fNodes) {
                // update velocities
                if (node.Fixed) {
                    node.v = new ArborPoint(0, 0);
                    node.f = new ArborPoint(0, 0);
                } else {
                    node.v = node.v.add(node.f.mul(dt));
                    node.v = node.v.mul(1 - param_friction);

                    node.f.x = node.f.y = 0;
                    double r = node.v.magnitude();
                    if (r > 1000) {
                        node.v = node.v.div(r * r);
                    }
                }

                // update positions
                node.Pt = node.Pt.add(node.v.mul(dt));
                double x = node.v.magnitude();
                double z = x * x;
                eSum += z;
                eMax = Math.Max(z, eMax);
            }

            energy_sum = eSum;
            energy_max = eMax;
            energy_mean = eSum / fNodes.Count;
        }

        internal static double NextRndDouble()
        {
            return _random.NextDouble();
        }
    }
}
