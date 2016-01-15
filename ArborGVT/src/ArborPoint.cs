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

        public static ArborPoint operator +(ArborPoint left, ArborPoint right) =>
            new ArborPoint(left.x + right.x, left.y + right.y);

        public static ArborPoint operator -(ArborPoint left, ArborPoint right) =>
            new ArborPoint(left.x - right.x, left.y - right.y);

        public static ArborPoint operator *(ArborPoint left, double right) =>
            new ArborPoint(left.x * right, left.y * right);

        public static ArborPoint operator /(ArborPoint left, double right) =>
            new ArborPoint(left.x / right, left.y / right);

        public static explicit operator bool(ArborPoint value)
        {
            return !(double.IsNaN(value.x) && double.IsNaN(value.y));
        }

        public static ArborPoint newRnd(double a = 5)
        {
            return new ArborPoint(
                2 * a * (ArborSystem.NextRndDouble() - 0.5),
                2 * a * (ArborSystem.NextRndDouble() - 0.5));
        }

        public bool exploded()
        {
            return (double.IsNaN(this.x) || double.IsNaN(this.y));
        }

        public double magnitude()
        {
            return Math.Sqrt(this.x * this.x + this.y * this.y);
        }

        public ArborPoint normalize()
        {
            return this / magnitude();
        }
    }
}
