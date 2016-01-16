//
//  Arbor - version 0.91
//  a graph vizualization toolkit
//
//  Copyright (c) 2011 Samizdat Drafting Co.
//  Physics code derived from springy.js, copyright (c) 2010 Dennis Hotson
// 

using System;

namespace ArborGVT
{
    public struct ArborPoint
    {
        public static readonly ArborPoint Null = new ArborPoint(double.NaN, double.NaN);
        
        public double X;
        public double Y;

        public ArborPoint(double x, double y)
        {
            this.X = x;
            this.Y = y;
        }

        public bool isNull()
        {
            return (double.IsNaN(this.X) && double.IsNaN(this.Y));
        }

        public static ArborPoint newRnd(double a = 5)
        {
            return new ArborPoint(2 * a * (ArborSystem.NextRndDouble() - 0.5), 2 * a * (ArborSystem.NextRndDouble() - 0.5));
        }

        public bool exploded()
        {
            return (double.IsNaN(this.X) || double.IsNaN(this.Y));
        }

        public ArborPoint add(ArborPoint a)
        {
            return new ArborPoint(this.X + a.X, this.Y + a.Y);
        }

        public ArborPoint sub(ArborPoint a)
        {
            return new ArborPoint(this.X - a.X, this.Y - a.Y);
        }

        public ArborPoint mul(double a)
        {
            return new ArborPoint(this.X * a, this.Y * a);
        }

        public ArborPoint div(double a)
        {
            return new ArborPoint(this.X / a, this.Y / a);
        }

        public double magnitude()
        {
            return Math.Sqrt(this.X * this.X + this.Y * this.Y);
        }

        public ArborPoint normalize()
        {
            return this.div(this.magnitude());
        }
    }
}
