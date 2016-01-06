//
//  Arbor - version 0.91
//  a graph vizualization toolkit
//
//  Copyright (c) 2011 Samizdat Drafting Co.
//  Physics code derived from springy.js, copyright (c) 2010 Dennis Hotson
// 

using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Windows.Forms;

namespace ArborGVT
{
    public sealed class ArborViewer : Panel, IArborRenderer
    {
        private bool fEnergyDebug;
        private ArborNode fDragged;
        private readonly Font fDrawFont;
        private readonly StringFormat fStrFormat;
        private readonly ArborSystem fSys;
        private readonly SolidBrush fWhiteBrush;

        public bool EnergyDebug
        {
            get { return this.fEnergyDebug; }
            set { this.fEnergyDebug = value; }
        }

        public ArborSystem Sys
        {
            get { return this.fSys; }
        }

        public ArborViewer()
        {
            base.BorderStyle = BorderStyle.Fixed3D;
            base.TabStop = true;
            base.BackColor = Color.White;

            base.DoubleBuffered = true;
            base.SetStyle(ControlStyles.AllPaintingInWmPaint, true);
            base.SetStyle(ControlStyles.OptimizedDoubleBuffer, true);

            // repulsion - отталкивание, stiffness - тугоподвижность, friction - сила трения
            this.fSys = new ArborSystem(10000, 250/*1000*/, 0.1, this);
            this.fSys.setScreenSize(this.Width, this.Height);
            this.fSys.AutoStop = false;

            this.fEnergyDebug = false;
            this.fDrawFont = new Font("Calibri", 9);

            this.fStrFormat = new StringFormat();
            this.fStrFormat.Alignment = StringAlignment.Center;
            this.fStrFormat.LineAlignment = StringAlignment.Center;

            this.fWhiteBrush = new SolidBrush(Color.White);
            this.fDragged = null;
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                this.fSys.Dispose();
                this.fDrawFont.Dispose();
            }
            base.Dispose(disposing);
        }

        protected override void OnResize(EventArgs e)
        {
            base.OnResize(e);

            this.fSys.setScreenSize(this.Width, this.Height);
            this.Invalidate();
        }

        protected override void OnPaint(PaintEventArgs pe)
        {
            Graphics gfx = pe.Graphics;

            try
            {
                gfx.SmoothingMode = SmoothingMode.AntiAlias;

                foreach (ArborNode node in fSys.Nodes)
                {
                    node.Box = this.getNodeRect(gfx, node);
                    gfx.FillRectangle(new SolidBrush(node.Color), node.Box);
                    gfx.DrawString(node.Sign, fDrawFont, this.fWhiteBrush, node.Box, this.fStrFormat);
                }

                using (Pen grayPen = new Pen(Color.Gray, 1))
                {
                    grayPen.StartCap = LineCap.NoAnchor;
                    grayPen.EndCap = LineCap.ArrowAnchor;

                    foreach (ArborEdge edge in fSys.Edges)
                    {
                        ArborNode srcNode = edge.Source;
                        ArborNode tgtNode = edge.Target;

                        ArborPoint pt1 = fSys.toScreen(srcNode.Pt);
                        ArborPoint pt2 = fSys.toScreen(tgtNode.Pt);

                        ArborPoint tail = intersect_line_box(pt1, pt2, srcNode.Box);
                        ArborPoint head = (tail.isNull()) ? ArborPoint.Null : intersect_line_box(tail, pt2, tgtNode.Box);

                        if (!head.isNull() && !tail.isNull()) {
                            gfx.DrawLine(grayPen, (int)tail.x, (int)tail.y, (int)head.x, (int)head.y);
                        }
                    }
                }

                if (this.fEnergyDebug) {
                    string energy = "max=" + fSys.energy_max.ToString("0.00000") + ", mean=" + fSys.energy_mean.ToString("0.00000") + ", thres=" + fSys.energy_threshold.ToString("0.00000");
                    gfx.DrawString(energy, fDrawFont, new SolidBrush(Color.Black), 10, 10);
                }
            } catch (Exception ex) {
                Debug.WriteLine("ArborViewer.OnPaint(): " + ex.Message);
            }
        }

        public static ArborPoint intersect_line_line(ArborPoint p1, ArborPoint p2, ArborPoint p3, ArborPoint p4)
        {
            double denom = ((p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y));
            if (denom == 0) return ArborPoint.Null; // lines are parallel

            double ua = ((p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x)) / denom;
            double ub = ((p2.x - p1.x) * (p1.y - p3.y) - (p2.y - p1.y) * (p1.x - p3.x)) / denom;

            if (ua < 0 || ua > 1 || ub < 0 || ub > 1) return ArborPoint.Null;

            return new ArborPoint(p1.x + ua * (p2.x - p1.x), p1.y + ua * (p2.y - p1.y));
        }

        public ArborPoint intersect_line_box(ArborPoint p1, ArborPoint p2, RectangleF boxTuple)
        {
            double bx = boxTuple.X;
            double by = boxTuple.Y;
            double bw = boxTuple.Width;
            double bh = boxTuple.Height;

            ArborPoint tl = new ArborPoint(bx, by);
            ArborPoint tr = new ArborPoint(bx + bw, by);
            ArborPoint bl = new ArborPoint(bx, by + bh);
            ArborPoint br = new ArborPoint(bx + bw, by + bh);

            ArborPoint pt;

            pt = intersect_line_line(p1, p2, tl, tr);
            if (!pt.isNull()) return pt;

            pt = intersect_line_line(p1, p2, tr, br);
            if (!pt.isNull()) return pt;

            pt = intersect_line_line(p1, p2, br, bl);
            if (!pt.isNull()) return pt;

            pt = intersect_line_line(p1, p2, bl, tl);
            if (!pt.isNull()) return pt;

            return ArborPoint.Null;
        }

        public void start()
        {
            this.fSys.start();
        }

        protected override void OnMouseDown(MouseEventArgs e)
        {
            base.OnMouseDown(e);
            if (!this.Focused) base.Focus();

            this.fDragged = fSys.nearest(e.X, e.Y);

            if (this.fDragged != null) {
                this.fDragged.Fixed = true;
            }
        }

        protected override void OnMouseUp(MouseEventArgs e)
        {
            base.OnMouseUp(e);
            
            if (this.fDragged != null) {
                this.fDragged.Fixed = false;
                //this.fDragged.Mass = 1000;
                this.fDragged = null;
            }
        }

        protected override void OnMouseMove(MouseEventArgs e)
        {
            base.OnMouseMove(e);
            
            if (this.fDragged != null) {
                this.fDragged.Pt = fSys.fromScreen(e.X, e.Y);
            }
        }

        public RectangleF getNodeRect(Graphics gfx, ArborNode node)
        {
            SizeF tsz = gfx.MeasureString(node.Sign, fDrawFont);
            float w = tsz.Width + 10;
            float h = tsz.Height + 4;
            ArborPoint pt = fSys.toScreen(node.Pt);
            pt.x = Math.Floor(pt.x);
            pt.y = Math.Floor(pt.y);

            return new RectangleF((float)pt.x - w / 2, (float)pt.y - h / 2, w, h);
        }

        public ArborNode getNodeByCoord(int x, int y)
        {
            return fSys.nearest(x, y);

            /*foreach (ArborNode node in fSys.Nodes)
            {
                if (node.Box.Contains(x, y)) {
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
}
