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

/**
 *
 * @author Serg V. Zhdanovskih
 */
public class ArborPoint
{
    public static final ArborPoint Null = new ArborPoint(Double.NaN, Double.NaN);

    public double X;
    public double Y;

    public ArborPoint(double x, double y)
    {
        this.X = x;
        this.Y = y;
    }

    public boolean isNull()
    {
        return (Double.isNaN(this.X) && Double.isNaN(this.Y));
    }

    public static ArborPoint newRnd(double a) // = 5
    {
        return new ArborPoint(2 * a * (ArborSystem.NextRndDouble() - 0.5), 2 * a * (ArborSystem.NextRndDouble() - 0.5));
    }

    public boolean exploded()
    {
        return (Double.isNaN(this.X) || Double.isNaN(this.Y));
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
        return Math.sqrt(this.X * this.X + this.Y * this.Y);
    }

    public double magnitudeSquare()
    {
        return this.X * this.X + this.Y * this.Y;
    }

    public ArborPoint normalize()
    {
        return this.div(this.magnitude());
    }
}
