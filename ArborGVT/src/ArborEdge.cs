﻿//
//  Arbor - version 0.91
//  a graph vizualization toolkit
//
//  Copyright (c) 2011 Samizdat Drafting Co.
//  Physics code derived from springy.js, copyright (c) 2010 Dennis Hotson
// 

using System;

namespace ArborGVT
{
    public class ArborEdge
    {
        public ArborNode Source;
        public ArborNode Target;

        public double Length;
        public bool Directed;
        public double Stiffness;

        public ArborEdge(ArborNode src, ArborNode tgt, double len, double stiffness)
        {
            this.Source = src;
            this.Target = tgt;
            this.Length = len;
            this.Directed = true;
            this.Stiffness = stiffness;
        }
    }
}
