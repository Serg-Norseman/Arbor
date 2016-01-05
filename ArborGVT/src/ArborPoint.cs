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
        
        public double x;
        public double y;

        public ArborPoint(double x, double y)
        {
            this.x = x;
            this.y = y;
        }

        public bool isNull()
        {
            return (double.IsNaN(this.x) && double.IsNaN(this.y));
        }

        public static ArborPoint newRnd(double a = 5)
        {
            return new ArborPoint(2 * a * (ArborSystem.NextRndDouble() - 0.5), 2 * a * (ArborSystem.NextRndDouble() - 0.5));
        }

        public bool exploded()
        {
            return (double.IsNaN(this.x) || double.IsNaN(this.y));
        }

        public ArborPoint add(ArborPoint a)
        {
            return new ArborPoint(this.x + a.x, this.y + a.y);
        }

        public ArborPoint sub(ArborPoint a)
        {
            return new ArborPoint(this.x - a.x, this.y - a.y);
        }

        public ArborPoint mul(double a)
        {
            return new ArborPoint(this.x * a, this.y * a);
        }

        public ArborPoint div(double a)
        {
            return new ArborPoint(this.x / a, this.y / a);
        }

        public double magnitude()
        {
            return Math.Sqrt(this.x * this.x + this.y * this.y);
        }

        public ArborPoint normalize()
        {
            return this.div(this.magnitude());
        }

        // not used
        /*public ArborPoint normal()
		{
            return new ArborPoint(-this.y, this.x);
        }*/
    }
}
