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

import arborgvt.ArborEdge;
import arborgvt.ArborNode;
import arborgvt.ArborPoint;
import arborgvt.ArborSystem;
import arborgvt.IArborRenderer;
import java.awt.Color;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionAdapter;
import java.awt.geom.Rectangle2D;
import javax.swing.BorderFactory;
import javax.swing.JPanel;

/**
 *
 * @author Serg V. Zhdanovskih
 */
public class ArborViewer extends JPanel implements IArborRenderer
{
    private boolean fEnergyDebug;
    private ArborNode fDragged;
    private boolean fNodesDragging;
    private final ArborSystemEx fSys;

    public boolean getEnergyDebug()
    {
        return this.fEnergyDebug;
    }

    public void setEnergyDebug(boolean value)
    {
        this.fEnergyDebug = value;
    }

    public boolean getNodesDragging()
    {
        return this.fNodesDragging;
    }

    public void setNodesDragging(boolean value)
    {
        this.fNodesDragging = value;
    }

    public ArborSystem getSystem()
    {
        return this.fSys;
    }

    public ArborViewer()
    {
        super();
        this.setBorder(BorderFactory.createLoweredBevelBorder());
        this.setDoubleBuffered(true);

        this.fSys = new ArborSystemEx(10000, 500/*1000*/, 0.1, this);
        this.fSys.setScreenSize(this.getWidth(), this.getHeight());
        this.fSys.setAutoStop(false);

        this.fEnergyDebug = false;
        this.fDragged = null;
        this.fNodesDragging = false;

        this.addMouseListener(new MouseAdapter()
        {
            @Override
            public void mousePressed(MouseEvent e)
            {
                onMouseDown(e);
            }

            @Override
            public void mouseReleased(MouseEvent e)
            {
                onMouseUp(e);
            }
        });

        this.addMouseMotionListener(new MouseMotionAdapter()
        {
            @Override
            public void mouseDragged(MouseEvent e)
            {
                onMouseMove(e);
            }
        });
    }

    @Override
    public void setBounds(int x, int y, int width, int height)
    {
        super.setBounds(x, y, width, height);

        this.fSys.setScreenSize(width, height);
        this.repaint();
    }

    @Override
    protected void paintComponent(Graphics g)
    {
        super.paintComponent(g);

        Graphics2D gfx = (Graphics2D) g;
        RenderingHints rh = new RenderingHints(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
        gfx.setRenderingHints(rh);

        FontMetrics metrics = gfx.getFontMetrics();

        try {
            for (ArborNode node : fSys.getNodes()) {
                ArborNodeEx nodeEx = (ArborNodeEx) node;
                nodeEx.Box = this.getNodeRect(gfx, node);

                gfx.setColor(nodeEx.Color);
                gfx.fillRect((int) nodeEx.Box.x, (int) nodeEx.Box.y, (int) nodeEx.Box.width, (int) nodeEx.Box.height);

                gfx.setColor(Color.white);
                gfx.drawString(node.Sign,
                        (int) (nodeEx.Box.x + (nodeEx.Box.width - metrics.stringWidth(node.Sign)) / 2),
                        (int) (nodeEx.Box.y + (nodeEx.Box.height - metrics.getHeight()) / 2 + metrics.getAscent()));
            }

            //using (Pen grayPen = new Pen(Color.Gray, 1))
            {
                //grayPen.StartCap = LineCap.NoAnchor;
                //grayPen.EndCap = LineCap.ArrowAnchor;
                gfx.setColor(Color.gray);

                for (ArborEdge edge : fSys.getEdges()) {
                    ArborNodeEx srcNode = (ArborNodeEx) edge.Source;
                    ArborNodeEx tgtNode = (ArborNodeEx) edge.Target;

                    ArborPoint pt1 = fSys.toScreen(srcNode.Pt);
                    ArborPoint pt2 = fSys.toScreen(tgtNode.Pt);

                    ArborPoint tail = intersect_line_box(pt1, pt2, srcNode.Box);
                    ArborPoint head = (tail.isNull()) ? ArborPoint.Null : intersect_line_box(tail, pt2, tgtNode.Box);

                    if (!head.isNull() && !tail.isNull()) {
                        gfx.drawLine((int) tail.X, (int) tail.Y, (int) head.X, (int) head.Y);
                    }
                }
            }

            if (this.fEnergyDebug) {
                gfx.setColor(Color.black);
                String energy = "max=" + String.format("%.5f", fSys.EnergyMax) + ", mean=" + String.format("%.5f", fSys.EnergyMean);
                gfx.drawString(energy, 10, 10 + metrics.getAscent());
            }
        } catch (Exception ex) {
            System.out.println("ArborViewer.OnPaint(): " + ex.getMessage());
        }
    }

    public static ArborPoint intersect_line_line(ArborPoint p1, ArborPoint p2, ArborPoint p3, ArborPoint p4)
    {
        double denom = ((p4.Y - p3.Y) * (p2.X - p1.X) - (p4.X - p3.X) * (p2.Y - p1.Y));
        if (denom == 0) {
            return ArborPoint.Null; // lines are parallel
        }
        double ua = ((p4.X - p3.X) * (p1.Y - p3.Y) - (p4.Y - p3.Y) * (p1.X - p3.X)) / denom;
        double ub = ((p2.X - p1.X) * (p1.Y - p3.Y) - (p2.Y - p1.Y) * (p1.X - p3.X)) / denom;

        if (ua < 0 || ua > 1 || ub < 0 || ub > 1) {
            return ArborPoint.Null;
        }

        return new ArborPoint(p1.X + ua * (p2.X - p1.X), p1.Y + ua * (p2.Y - p1.Y));
    }

    public ArborPoint intersect_line_box(ArborPoint p1, ArborPoint p2, Rectangle2D.Float boxTuple)
    {
        double bx = boxTuple.x;
        double by = boxTuple.y;
        double bw = boxTuple.width;
        double bh = boxTuple.height;

        ArborPoint tl = new ArborPoint(bx, by);
        ArborPoint tr = new ArborPoint(bx + bw, by);
        ArborPoint bl = new ArborPoint(bx, by + bh);
        ArborPoint br = new ArborPoint(bx + bw, by + bh);

        ArborPoint pt;

        pt = intersect_line_line(p1, p2, tl, tr);
        if (!pt.isNull()) {
            return pt;
        }

        pt = intersect_line_line(p1, p2, tr, br);
        if (!pt.isNull()) {
            return pt;
        }

        pt = intersect_line_line(p1, p2, br, bl);
        if (!pt.isNull()) {
            return pt;
        }

        pt = intersect_line_line(p1, p2, bl, tl);
        if (!pt.isNull()) {
            return pt;
        }

        return ArborPoint.Null;
    }

    public void start()
    {
        this.fSys.start();
    }

    private void onMouseDown(MouseEvent e)
    {
        if (this.fNodesDragging) {
            this.fDragged = fSys.nearest(e.getX(), e.getY());

            if (this.fDragged != null) {
                this.fDragged.Fixed = true;
            }
        }
    }

    private void onMouseUp(MouseEvent e)
    {
        if (this.fNodesDragging && this.fDragged != null) {
            this.fDragged.Fixed = false;
            //this.fDragged.Mass = 1000;
            this.fDragged = null;
        }
    }

    private void onMouseMove(MouseEvent e)
    {
        if (this.fNodesDragging && this.fDragged != null) {
            this.fDragged.Pt = fSys.fromScreen(e.getX(), e.getY());
        }
    }

    public Rectangle2D.Float getNodeRect(Graphics gfx, ArborNode node)
    {
        FontMetrics metrics = gfx.getFontMetrics();
        int tw = metrics.stringWidth(node.Sign);
        int th = metrics.getHeight();

        float w = tw + 10;
        float h = th + 4;
        ArborPoint pt = fSys.toScreen(node.Pt);

        return new Rectangle2D.Float((float) (pt.X - w / 2), (float) (pt.Y - h / 2), w, h);
    }

    public ArborNode getNodeByCoord(int x, int y)
    {
        return fSys.nearest(x, y);

        /*for (ArborNode node : fSys.getNodes()) {
            if (node.Box.contains(x, y)) {
                return node;
            }
        }
        return null;*/
    }

    public void doSample()
    {
        fSys.addEdge("1", "4", 2);
        fSys.addEdge("1", "12");
        fSys.addEdge("4", "21");
        fSys.addEdge("4", "23");
        fSys.addEdge("7", "34");
        fSys.addEdge("7", "13");
        fSys.addEdge("7", "44");
        fSys.addEdge("12", "25");
        fSys.addEdge("12", "24");
        fSys.addEdge("23", "50");
        fSys.addEdge("23", "53");
        fSys.addEdge("24", "6");
        fSys.addEdge("24", "42");
        fSys.addEdge("25", "94");
        fSys.addEdge("25", "66");
        fSys.addEdge("32", "47");
        fSys.addEdge("32", "84");
        fSys.addEdge("42", "32");
        fSys.addEdge("42", "7");
        fSys.addEdge("50", "72");
        fSys.addEdge("50", "65");
        fSys.addEdge("53", "67");
        fSys.addEdge("53", "68");
        fSys.addEdge("66", "79");
        fSys.addEdge("66", "80");
        fSys.addEdge("67", "88");
        fSys.addEdge("67", "83");
        fSys.addEdge("68", "77");
        fSys.addEdge("68", "91");
        fSys.addEdge("80", "99");
        fSys.addEdge("80", "97");
        fSys.addEdge("88", "110");
        fSys.addEdge("88", "104");
        fSys.addEdge("91", "106");
        fSys.addEdge("91", "100");
    }
}
