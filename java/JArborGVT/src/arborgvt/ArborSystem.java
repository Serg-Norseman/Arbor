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

import java.awt.event.ActionEvent;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.Random;
import javax.swing.Timer;

/**
 *
 * @author Serg V. Zhdanovskih
 */
public class ArborSystem
{
    public class PSBounds
    {
        public ArborPoint LeftTop = ArborPoint.Null;
        public ArborPoint RightBottom = ArborPoint.Null;

        public PSBounds(ArborPoint leftTop, ArborPoint rightBottom)
        {
            this.LeftTop = leftTop;
            this.RightBottom = rightBottom;
        }
    }

    private static final int DEBUG_PROFILER_LIMIT = 0;//2000;

    private static final Random _random = new Random();

    private final int[] margins = new int[]{20, 20, 20, 20};
    private static final double Mag = 0.04;

    private boolean fAutoStop;
    private boolean fBusy;
    private final List<ArborEdge> fEdges;
    private PSBounds fGraphBounds;
    private int fIterationsCounter;
    private final Hashtable<String, ArborNode> fNames;
    private final List<ArborNode> fNodes;
    //private EventHandler fOnStart;
    //private EventHandler fOnStop;
    private final IArborRenderer fRenderer;
    private long fPrevTime;
    private int fScreenHeight;
    private int fScreenWidth;
    private double fStopThreshold;
    private Timer fTimer;
    private PSBounds fViewBounds;

    public double EnergySum = 0;
    public double EnergyMax = 0;
    public double EnergyMean = 0;

    public double ParamRepulsion = 1000;
    public double ParamStiffness = 600;
    public double ParamFriction = 0.5;
    public double ParamDt = 0.01; // 0.02;
    public boolean ParamGravity = false;
    public double ParamPrecision = 0.6;
    public double ParamTimeout = 1000 / 100;
    public double ParamTheta = 0.4;

    public boolean getAutoStop()
    {
        return this.fAutoStop;
    }

    public void setAutoStop(boolean value)
    {
        this.fAutoStop = value;
    }

    public List<ArborNode> getNodes()
    {
        return this.fNodes;
    }

    public List<ArborEdge> getEdges()
    {
        return this.fEdges;
    }

    /*public event EventHandler OnStart
        {
            add
            {
                this.fOnStart = value;
            }
            remove
            {
                if (this.fOnStart == value)
                {
                    this.fOnStart = null;
                }
            }
        }

        public event EventHandler OnStop
        {
            add
            {
                this.fOnStop = value;
            }
            remove
            {
                if (this.fOnStop == value)
                {
                    this.fOnStop = null;
                }
            }
        }*/

    public double getStopThreshold()
    {
        return this.fStopThreshold;
    }

    public void setStopThreshold(double value)
    {
        this.fStopThreshold = value;
    }

    public ArborSystem(double repulsion, double stiffness, double friction, IArborRenderer renderer)
    {
        this.fAutoStop = true;
        this.fBusy = false;
        this.fNames = new Hashtable();
        this.fNodes = new ArrayList<>();
        this.fEdges = new ArrayList<>();
        this.fRenderer = renderer;
        this.fPrevTime = 0;
        this.fStopThreshold = /*0.05*/ 0.7;
        this.fTimer = null;

        this.ParamRepulsion = repulsion;
        this.ParamStiffness = stiffness;
        this.ParamFriction = friction;
    }

    public void start()
    {
        //if (fOnStart != null) fOnStart(this, new EventArgs());

        if (fTimer != null) {
            return;
        }
        fPrevTime = 0;

        fIterationsCounter = 0;

        fTimer = new Timer((int) ParamTimeout, (ActionEvent e) -> {
            long time = System.currentTimeMillis();
            this.tickTimer(time);
        });
        fTimer.setRepeats(true);
        fTimer.start();
    }

    public void stop()
    {
        if (fTimer != null) {
            fTimer.stop();
            //fTimer.Dispose();
            fTimer = null;
        }

        //if (fOnStop != null) fOnStop(this, new EventArgs());
    }

    public ArborNode addNode(String sign, double x, double y)
    {
        ArborNode node = this.getNode(sign);
        if (node != null) {
            return node;
        }

        node = new ArborNode(sign);
        node.Pt = new ArborPoint(x, y);

        fNames.put(sign, node);
        fNodes.add(node);

        return node;
    }

    public ArborNode addNode(String sign)
    {
        ArborPoint lt = this.fGraphBounds.LeftTop;
        ArborPoint rb = this.fGraphBounds.RightBottom;
        double xx = lt.X + (rb.X - lt.X) * ArborSystem.NextRndDouble();
        double yy = lt.Y + (rb.Y - lt.Y) * ArborSystem.NextRndDouble();

        return this.addNode(sign, xx, yy);
    }

    public ArborNode getNode(String sign)
    {
        return (ArborNode) fNames.get(sign);
    }

    public ArborEdge addEdge(String srcSign, String tgtSign)
    {
        return this.addEdge(srcSign, tgtSign, 1);
    }

    public ArborEdge addEdge(String srcSign, String tgtSign, int len)
    {
        ArborNode src = this.getNode(srcSign);
        src = (src != null) ? src : this.addNode(srcSign);

        ArborNode tgt = this.getNode(tgtSign);
        tgt = (tgt != null) ? tgt : this.addNode(tgtSign);

        ArborEdge x = null;
        if (src != null && tgt != null) {
            for (ArborEdge edge : fEdges) {
                if (edge.Source == src && edge.Target == tgt) {
                    x = edge;
                    break;
                }
            }
        }

        if (x == null) {
            x = new ArborEdge(src, tgt, len, ParamStiffness);
            fEdges.add(x);
        }

        return x;
    }

    public void setScreenSize(int width, int height)
    {
        this.fScreenWidth = width;
        this.fScreenHeight = height;
        this.updateViewBounds();
    }

    public ArborPoint toScreen(ArborPoint pt)
    {
        if (fViewBounds == null) {
            return ArborPoint.Null;
        }

        ArborPoint vd = fViewBounds.RightBottom.sub(fViewBounds.LeftTop);
        double sx = margins[3] + pt.sub(fViewBounds.LeftTop).div(vd.X).X * (this.fScreenWidth - (margins[1] + margins[3]));
        double sy = margins[0] + pt.sub(fViewBounds.LeftTop).div(vd.Y).Y * (this.fScreenHeight - (margins[0] + margins[2]));
        return new ArborPoint(sx, sy);
    }

    public ArborPoint fromScreen(double sx, double sy)
    {
        if (fViewBounds == null) {
            return ArborPoint.Null;
        }

        ArborPoint vd = fViewBounds.RightBottom.sub(fViewBounds.LeftTop);
        double x = (sx - margins[3]) / (this.fScreenWidth - (margins[1] + margins[3])) * vd.X + fViewBounds.LeftTop.X;
        double y = (sy - margins[0]) / (this.fScreenHeight - (margins[0] + margins[2])) * vd.Y + fViewBounds.LeftTop.Y;
        return new ArborPoint(x, y);
    }

    public ArborNode nearest(int sx, int sy)
    {
        ArborPoint x = this.fromScreen(sx, sy);

        ArborNode resNode = null;
        double minDist = +1.0;

        for (ArborNode node : this.fNodes) {
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

    private void updateGraphBounds()
    {
        ArborPoint lt = new ArborPoint(-1, -1);
        ArborPoint rb = new ArborPoint(1, 1);

        for (ArborNode node : this.fNodes) {
            ArborPoint pt = node.Pt;
            if (pt.exploded()) {
                continue;
            }

            if (pt.X < lt.X) {
                lt.X = pt.X;
            }
            if (pt.Y < lt.Y) {
                lt.Y = pt.Y;
            }
            if (pt.X > rb.X) {
                rb.X = pt.X;
            }
            if (pt.Y > rb.Y) {
                rb.Y = pt.Y;
            }
        }

        lt.X -= 1.2;
        lt.Y -= 1.2;
        rb.X += 1.2;
        rb.Y += 1.2;

        ArborPoint sz = rb.sub(lt);
        ArborPoint cent = lt.add(sz.div(2));
        ArborPoint d = new ArborPoint(Math.max(sz.X, 4.0), Math.max(sz.Y, 4.0)).div(2);

        this.fGraphBounds = new PSBounds(cent.sub(d), cent.add(d));
    }

    private void updateViewBounds()
    {
        try {
            this.updateGraphBounds();

            if (fViewBounds == null) {
                fViewBounds = fGraphBounds;
                return;
            }

            ArborPoint nbLT = fViewBounds.LeftTop.add(fGraphBounds.LeftTop.sub(fViewBounds.LeftTop).mul(Mag));
            ArborPoint nbRB = fViewBounds.RightBottom.add(fGraphBounds.RightBottom.sub(fViewBounds.RightBottom).mul(Mag));

            double aX = fViewBounds.LeftTop.sub(nbLT).magnitude() * this.fScreenWidth;
            double aY = fViewBounds.RightBottom.sub(nbRB).magnitude() * this.fScreenHeight;

            if (aX > 1 || aY > 1) {
                fViewBounds = new PSBounds(nbLT, nbRB);
            }
        } catch (Exception ex) {
            System.out.println("ArborSystem.updateViewBounds(): " + ex.getMessage());
        }
    }

    private void tickTimer(long time)
    {
        if (DEBUG_PROFILER_LIMIT > 0) {
            if (fIterationsCounter >= DEBUG_PROFILER_LIMIT) {
                return;
            } else {
                fIterationsCounter++;
            }
        }

        if (this.fBusy) {
            return;
        }
        this.fBusy = true;
        try {
            this.updatePhysics();
            this.updateViewBounds();

            if (fRenderer != null) {
                fRenderer.repaint();
            }

            if (this.fAutoStop) {
                if (EnergyMean <= this.fStopThreshold) {
                    if (fPrevTime == 0) {
                        fPrevTime = time;
                    }
                    if (time - fPrevTime > 1000) {
                        this.stop();
                    }
                } else {
                    fPrevTime = 0;
                }
            }
        } catch (Exception ex) {
            System.out.println("ArborSystem.tickTimer(): " + ex.getMessage());
        }
        this.fBusy = false;
    }

    private void updatePhysics()
    {
        try {
            // tend particles
            for (ArborNode p : fNodes) {
                p.V.X = 0;
                p.V.Y = 0;
            }

            if (ParamStiffness > 0) {
                this.applySprings();
            }

            // euler integrator
            if (ParamRepulsion > 0) {
                this.applyBarnesHutRepulsion();
            }

            this.updateVelocityAndPosition(ParamDt);
        } catch (Exception ex) {
            System.out.println("ArborSystem.updatePhysics(): " + ex.getMessage());
        }
    }

    private void applyBarnesHutRepulsion()
    {
        BarnesHutTree bht = new BarnesHutTree(fGraphBounds.LeftTop, fGraphBounds.RightBottom, ParamTheta);

        for (ArborNode node : fNodes) {
            bht.insert(node);
        }

        for (ArborNode node : fNodes) {
            bht.applyForces(node, ParamRepulsion);
        }
    }

    private void applySprings()
    {
        for (ArborEdge edge : fEdges) {
            ArborPoint s = edge.Target.Pt.sub(edge.Source.Pt);
            double sMag = s.magnitude();

            ArborPoint r = ((sMag > 0) ? s : ArborPoint.newRnd(1)).normalize();
            double q = edge.Stiffness * (edge.Length - sMag);

            edge.Source.applyForce(r.mul(q * -0.5));
            edge.Target.applyForce(r.mul(q * 0.5));
        }
    }

    private void updateVelocityAndPosition(double dt)
    {
        int size = fNodes.size();
        if (size == 0) {
            EnergySum = 0;
            EnergyMax = 0;
            EnergyMean = 0;
            return;
        }

        double eSum = 0;
        double eMax = 0;

        // calc center drift
        ArborPoint rr = new ArborPoint(0, 0);
        for (ArborNode node : fNodes) {
            rr = rr.add(node.Pt);
        }
        ArborPoint drift = rr.div(-size);

        // main updates loop
        for (ArborNode node : fNodes) {
            // apply center drift
            node.applyForce(drift);

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
            eMax = Math.max(z, eMax);
        }

        EnergySum = eSum;
        EnergyMax = eMax;
        EnergyMean = eSum / size;
    }

    public static double NextRndDouble()
    {
        return _random.nextDouble();
    }
}
