//
//  Arbor - version 0.91
//  a graph vizualization toolkit
//
//  Copyright (c) 2011 Samizdat Drafting Co.
//  Physics code derived from springy.js, copyright (c) 2010 Dennis Hotson
// 
(function () {

    var ext = {
        each:function (d, e) {
            for (var a in d) {
                e(a, d[a]);
            }
        }
    };


    var Node = function (name) {
        this._id = _nextNodeId++;
		this.name = "" + name;
        this.mass = 1;
        this._fixed = false;
        this._p = new ArbPoint(NaN, NaN);
		this.color = "#cccccc";
    };

    var _nextNodeId = 1;

    var Edge = function (b, c) {
        this._id = _nextEdgeId--;
        this.source = b;
        this.target = c;
        this.length = 1;
		this.directed = true;
    };

    var _nextEdgeId = -1;

    var Particle = function (a, b, f) {
        this.p = a;
        this.m = b;
        this.v = new ArbPoint(0, 0);
        this.f = new ArbPoint(0, 0);
		this.fixed = f;
		this.name = "particle";
    };

    Particle.prototype.applyForce = function (a) {
        this.f = this.f.add(a.divide(this.m));
    };

    var Spring = function (c, b, d, a) {
        this.point1 = c;
        this.point2 = b;
        this.length = d;
        this.k = a;
    };

    var ArbPoint = function (a, b) {
        this.x = a;
        this.y = b;
    };

    ArbPoint.prototype = {
		rnd:function (a) {
			a = (a !== undefined) ? a : 5;
			return new ArbPoint(2 * a * (Math.random() - 0.5), 2 * a * (Math.random() - 0.5));
		},
        exploded:function () {
            return (isNaN(this.x) || isNaN(this.y));
        },
        add:function (a) {
            return new ArbPoint(this.x + a.x, this.y + a.y);
        },
        subtract:function (a) {
            return new ArbPoint(this.x - a.x, this.y - a.y);
        },
        multiply:function (a) {
            return new ArbPoint(this.x * a, this.y * a);
        },
        divide:function (a) {
            return new ArbPoint(this.x / a, this.y / a);
        },
        magnitude:function () {
            return Math.sqrt(this.x * this.x + this.y * this.y);
        },
        normal:function () {
            return new ArbPoint(-this.y, this.x);
        },
        normalize:function () {
            return this.divide(this.magnitude());
        }
    };


    var ParticleSystem = function (d, p, e, renderer) {
        var usz = null;
        var mag = 0.04;
        var margins = [20, 20, 20, 20];
        var n_bnd = null;
        var o_bnd = null;

        var param_repulsion = isNaN(d) ? 1000 : d;
        var param_stiffness = isNaN(p) ? 600 : p;
        var param_friction = isNaN(e) ? 0.5 : e;
        var param_dt = 0.02;
        var param_gravity = false;
        var param_precision = 0.6;
        var param_timeout = 1000 / 55;

        var c_renderer = renderer;
        var c_nodes = [];
        var c_edges = [];
        var c_names = [];

        var itv = null;
        var tm = null;
		
        var bht = BarnesHutTree();
        var c_particles = [];
        var c_springs = [];
        var gdt_topleft = new ArbPoint(-1, -1);
        var gdt_bottomright = new ArbPoint(1, 1);
		var theta = 0.4;

        var energy_sum = 0;
        var energy_max = 0;
        var energy_mean = 0;

        var that = {
            init:function () {
                return that;
            },

            physicsUpdate:function () {
                that.tick();
                that._updateBounds();

                if (c_renderer !== undefined) {
                    c_renderer.redraw();
                }

                if ((energy_mean + energy_max) / 2 < 0.05) {
                    if (tm === null) {
                        tm = new Date().valueOf();
                    }
                    if (new Date().valueOf() - tm > 1000) {
                        that.stop();
                    }
                } else {
                    tm = null;
                }
            },

            start:function () {
                if (itv !== null) {
                    return;
                }
                tm = null;
                itv = setInterval(that.physicsUpdate, param_timeout);
            },

            stop:function () {
                if (itv !== null) {
                    clearInterval(itv);
                    itv = null;
                }
            },

            addNode:function (w) {
                var C = that.getNode(w);
                if (C) {
                    return C;
                } else {
                    var z = new Node(w);
                    c_names[w] = z;
                    c_nodes[z._id] = z;

                    that.addParticle(z._id, z.mass, NaN, NaN, z._fixed);
                    return z;
                }
            },

            getNode:function (v) {
				return c_names[v];
            },

            addEdge:function (z, A) {
                var src = that.getNode(z) || that.addNode(z);
                var tgt = that.getNode(A) || that.addNode(A);

				var x = null;
                if (src && tgt) {
					for (var edge in c_edges) {
						if (edge.source === src && edge.target === tgt) {
							x = edge;
							break;
						}
					}
                }

                if (x === null) {
					x = new Edge(src, tgt);
                    c_edges[x._id] = x;
					that.addSpring(x._id, src._id, tgt._id, x.length);
                }
                return x;
            },

            eachNode:function (v) {
                ext.each(c_nodes, function (y, x) {
                    if (x._p.x === null || x._p.y === null) {
                        return;
                    }
                    var w = (usz !== null) ? that.toScreen(x._p) : x._p;
                    v.call(that, x, w);
                });
            },

            eachEdge:function (v) {
                ext.each(c_edges, function (z, x) {
                    var y = c_nodes[x.source._id]._p;
                    var w = c_nodes[x.target._id]._p;
                    if (y.x === null || w.x === null) {
                        return;
                    }
                    y = (usz !== null) ? that.toScreen(y) : y;
                    w = (usz !== null) ? that.toScreen(w) : w;
                    if (y && w) {
                        v.call(that, x, y, w);
                    }
                });
            },

            screenSize:function (v, w) {
                usz = {
                    width:v,
                    height:w
                };
                that._updateBounds();
            },

            toScreen:function (x) {
                if (!n_bnd || !usz) {
                    return;
                }
                var v = n_bnd.bottomright.subtract(n_bnd.topleft);
                var z = margins[3] + x.subtract(n_bnd.topleft).divide(v.x).x * (usz.width - (margins[1] + margins[3]));
                var y = margins[0] + x.subtract(n_bnd.topleft).divide(v.y).y * (usz.height - (margins[0] + margins[2]));
                return new ArbPoint(z, y);
            },

            fromScreen:function (z) {
                if (!n_bnd || !usz) {
                    return;
                }
                var x = n_bnd.bottomright.subtract(n_bnd.topleft);
                var w = (z.x - margins[3]) / (usz.width - (margins[1] + margins[3])) * x.x + n_bnd.topleft.x;
                var v = (z.y - margins[0]) / (usz.height - (margins[0] + margins[2])) * x.y + n_bnd.topleft.y;
                return new ArbPoint(w, v);
            },

            _updateBounds:function () {
                if (usz === null) return;

				o_bnd = that.bounds();

                var z = new ArbPoint(o_bnd.bottomright.x, o_bnd.bottomright.y);
                var y = new ArbPoint(o_bnd.topleft.x, o_bnd.topleft.y);
                var B = z.subtract(y);
                var v = y.add(B.divide(2));
                var x = 4;

                var D = new ArbPoint(Math.max(B.x, x), Math.max(B.y, x));
                o_bnd.topleft = v.subtract(D.divide(2));
                o_bnd.bottomright = v.add(D.divide(2));

                if (!n_bnd) {
                    n_bnd = o_bnd;
                    return;
                }

                var _nb_BR = n_bnd.bottomright.add(o_bnd.bottomright.subtract(n_bnd.bottomright).multiply(mag));
                var _nb_TL = n_bnd.topleft.add(o_bnd.topleft.subtract(n_bnd.topleft).multiply(mag));

                var A = new ArbPoint(n_bnd.topleft.subtract(_nb_TL).magnitude(), n_bnd.bottomright.subtract(_nb_BR).magnitude());

                if (A.x * usz.width > 1 || A.y * usz.height > 1) {
                    n_bnd = {
                        topleft: _nb_TL,
                        bottomright: _nb_BR
                    };
                    return;
                } else {
					console.log("ok4");
                    return;
                }
            },

            bounds:function () {
                var w = null;
                var v = null;
                ext.each(c_nodes, function (z, node) {
                    var pt = node._p;
                    if (pt.exploded()) return;

                    if (w === null) {
                        w = new ArbPoint(pt.x, pt.y);
                        v = new ArbPoint(pt.x, pt.y);
                    }

                    if (pt.x > w.x) w.x = pt.x;
                    if (pt.y > w.y) w.y = pt.y;
                    if (pt.x < v.x) v.x = pt.x;
                    if (pt.y < v.y) v.y = pt.y;
                });

                if (!w || !v) {
                    w = new ArbPoint(-1, -1);
                    v = new ArbPoint(1, 1);
				}

                return { topleft: v, bottomright: w };
            },

            addParticle:function (id, m, x, y, f) {
                var xx = gdt_topleft.x + (gdt_bottomright.x - gdt_topleft.x) * Math.random();
                var yy = gdt_topleft.y + (gdt_bottomright.y - gdt_topleft.y) * Math.random();
                var r = new ArbPoint(!isNaN(x) ? x : xx, !isNaN(y) ? y : yy);
                c_particles[id] = new Particle(r, m, f);
            },

            dropParticle:function (s) {
                delete c_particles[s.id];
            },

            addSpring:function (id, fm, to, len) {
                var r = c_particles[fm];
                var q = c_particles[to];
                if (r !== undefined && q !== undefined) {
                    c_springs[id] = new Spring(r, q, len, param_stiffness);
                }
            },

            dropSpring:function (s) {
                delete c_springs[s.id];
            },

            updateGeometry:function () {
                ext.each(c_particles, function (r, q) {
                    c_nodes[r]._p.x = q.p.x;
                    c_nodes[r]._p.y = q.p.y;
                });
            },

            tick:function () {
                that.tendParticles();
                that.eulerIntegrator();
                that.updateGeometry();
            },

            tendParticles:function () {
                ext.each(c_particles, function (q, p) {
                    p.v.x = 0;
					p.v.y = 0;
                });
            },

            eulerIntegrator:function () {
                if (param_repulsion > 0) {
                    if (theta > 0) {
                        that.applyBarnesHutRepulsion();
                    } else {
                        that.applyBruteForceRepulsion();
                    }
                }

                if (param_stiffness > 0) {
                    that.applySprings();
                }

                that.applyCenterDrift();

                if (param_gravity) {
                    that.applyCenterGravity();
                }

                that.updateVelocity(param_dt);
                that.updatePosition(param_dt);
            },

            applyBruteForceRepulsion:function () {
                ext.each(c_particles, function (q, p) {
                    ext.each(c_particles, function (s, r) {
                        if (p !== r) {
                            var u = p.p.subtract(r.p);
                            var v = Math.max(1, u.magnitude());
                            var t = ((u.magnitude() > 0) ? u : ArbPoint.rnd(1)).normalize();
                            p.applyForce(t.multiply(param_repulsion * r.m * 0.5).divide(v * v * 0.5));
                            r.applyForce(t.multiply(param_repulsion * p.m * 0.5).divide(v * v * -0.5));
                        }
                    });
                });
            },

            applyBarnesHutRepulsion:function () {
                if (!gdt_topleft || !gdt_bottomright) {
                    return;
                }

                bht.init(gdt_topleft, gdt_bottomright, theta);
                ext.each(c_particles, function (s, r) {
                    bht.insert(r);
                });

                ext.each(c_particles, function (s, r) {
                    bht.applyForces(r, param_repulsion);
                });
            },

            applySprings:function () {
                ext.each(c_springs, function (t, p) {
                    var s = p.point2.p.subtract(p.point1.p);
                    var q = p.length - s.magnitude();
                    var r = ((s.magnitude() > 0) ? s : ArbPoint.rnd(1)).normalize();
                    p.point1.applyForce(r.multiply(p.k * q * -0.5));
                    p.point2.applyForce(r.multiply(p.k * q * 0.5));
                });
            },

            applyCenterDrift:function () {
                var q = 0;
                var r = new ArbPoint(0, 0);
                ext.each(c_particles, function (t, s) {
                    r.add(s.p);
                    q++;
                });
                if (q === 0) {
                    return;
                }
                var p = r.divide(-q);
                ext.each(c_particles, function (t, s) {
                    s.applyForce(p);
                });
            },

            applyCenterGravity:function () {
                ext.each(c_particles, function (r, p) {
                    var q = p.p.multiply(-1);
                    p.applyForce(q.multiply(param_repulsion / 100));
                });
            },

            updateVelocity:function (p) {
                ext.each(c_particles, function (t, q) {
                    if (q.fixed) {
                        q.v = new ArbPoint(0, 0);
                        q.f = new ArbPoint(0, 0);
                        return;
                    }
                    q.v = q.v.add(q.f.multiply(p)).multiply(1 - param_friction);
                    q.f.x = q.f.y = 0;
                    var r = q.v.magnitude();
                    if (r > 1000) {
                        q.v = q.v.divide(r * r);
                    }
                });
            },

            updatePosition:function (q) {
                var r = 0;
                var p = 0;
                var u = 0;
                var t = null;
                var s = null;

                ext.each(c_particles, function (w, v) {
                    v.p = v.p.add(v.v.multiply(q));
                    var x = v.v.magnitude();
                    var z = x * x;
                    r += z;
                    p = Math.max(z, p);
                    u++;

                    if (!t) {
                        t = new ArbPoint(v.p.x, v.p.y);
                        s = new ArbPoint(v.p.x, v.p.y);
                        return;
                    }

                    var y = v.p;
                    if (y.x === null || y.y === null) {
                        return;
                    }

                    if (y.x > t.x) t.x = y.x;
                    if (y.y > t.y) t.y = y.y;
                    if (y.x < s.x) s.x = y.x;
                    if (y.y < s.y) s.y = y.y;
                });

                energy_sum = r;
                energy_max = p;
                energy_mean = r / u;

                gdt_topleft = s || new ArbPoint(-1, -1);
                gdt_bottomright = t || new ArbPoint(1, 1);
            }

        };

        return that.init();
    };


    var BarnesHutTree = function () {
        var root = null;
        var d = 0.5;

        var that = {
            init:function (g, h, f) {
                d = f;
                root = that._newBranch();
                root.origin = g;
                root.size = h.subtract(g);
            },

            _whichQuad:function (i, f) {
                if (i.p.exploded()) {
                    return null;
                }
                var h = i.p.subtract(f.origin);
                var g = f.size.divide(2);
                if (h.y < g.y) {
                    if (h.x < g.x) {
                        return "nw";
                    } else {
                        return "ne";
                    }
                } else {
                    if (h.x < g.x) {
                        return "sw";
                    } else {
                        return "se";
                    }
                }
            },

            _newBranch:function () {
                var f = {};
				f.name = "branch";
                f.origin = null;
                f.size = null;
                f.nw = null;
                f.ne = null;
                f.sw = null;
                f.se = null;
                f.mass = 0;
                return f;
            },

            insert:function (j) {
                var f = root;
                var gst = [];
				gst.push(j);
                while (gst.length) {
                    var h = gst.shift();

                    var m = h.m;
                    var p = that._whichQuad(h, f);
					var fp = f[p];
					
                    if (fp === null) {
                        f[p] = h;

                        f.mass += m;
                        if (f.pt) {
                            f.pt = f.pt.add(h.p.multiply(m));
                        } else {
                            f.pt = h.p.multiply(m);
                        }
                    } else {
                        if ("origin" in fp) {
							/* only branches */
                            f.mass += (m);
                            if (f.pt) {
                                f.pt = f.pt.add(h.p.multiply(m));
                            } else {
                                f.pt = h.p.multiply(m);
                            }

                            f = fp;

                            gst.unshift(h);
                        } else {
							//console.log("fp: "+fp.name);
							/* only particle */
                            var l = f.size.divide(2);
                            var n = new ArbPoint(f.origin.x, f.origin.y);

							// ne, nw, se, sw
                            if (p === "se" || p === "sw") {
                                n.y += l.y;
                            }
                            if (p === "ne" || p === "se") {
                                n.x += l.x;
                            }

                            var o = fp;
							fp = that._newBranch();
                            fp.origin = n;
                            fp.size = l;
                            f[p] = fp;

                            f.mass = m;
                            f.pt = h.p.multiply(m);

                            f = fp;

                            if (o.p.x === h.p.x && o.p.y === h.p.y) {
                                var k = l.x * 0.08;
                                var i = l.y * 0.08;
                                o.p.x = Math.min(n.x + l.x, Math.max(n.x, o.p.x - k / 2 + Math.random() * k));
                                o.p.y = Math.min(n.y + l.y, Math.max(n.y, o.p.y - i / 2 + Math.random() * i));
                            }

                            gst.push(o);
                            gst.unshift(h);
                        }
                    }
                }
            },

            applyForces:function (m, g) {
                var f = [];
				f.push(root);
                while (f.length) {
                    var node = f.shift();
                    if (node === null || node === m) continue;

                    var k;
                    var l;
                    var i;
                    if ("f" in node) {
						/* particle */
                        k = m.p.subtract(node.p);
                        l = Math.max(1, k.magnitude());
                        i = ((k.magnitude() > 0) ? k : ArbPoint.rnd(1)).normalize();
                        m.applyForce(i.multiply(g * node.m).divide(l * l));
                    } else {
						/* branch */
                        var j = m.p.subtract(node.pt.divide(node.mass)).magnitude();
                        var h = Math.sqrt(node.size.x * node.size.y);
                        if (h / j > d) {
                            f.push(node.ne);
                            f.push(node.nw);
                            f.push(node.se);
                            f.push(node.sw);
                        } else {
                            k = m.p.subtract(node.pt.divide(node.mass));
                            l = Math.max(1, k.magnitude());
                            i = ((k.magnitude() > 0) ? k : ArbPoint.rnd(1)).normalize();
                            m.applyForce(i.multiply(g * (node.mass)).divide(l * l));
                        }
                    }
                }
            }
        };

        return that;
    };


    var HalfViz = function () {
        var _canvas = document.getElementById('viewport');
        var ctx = _canvas.getContext("2d");
        var sys = null;

        var that = {
            init:function () {
				sys = ParticleSystem(10000, 1000, 0.1, that);
                sys.screenSize(_canvas.width, _canvas.height);

				sys.addEdge("1", "4");
				sys.addEdge("1", "12");
				sys.addEdge("4", "21");
				sys.addEdge("4", "23");
				sys.addEdge("7", "34");
				sys.addEdge("7", "13");
				sys.addEdge("7", "44");
				sys.addEdge("12", "25");
				sys.addEdge("12", "24");
				sys.addEdge("23", "50");
				sys.addEdge("23", "53");
				sys.addEdge("24", "6");
				sys.addEdge("24", "42");
				sys.addEdge("25", "94");
				sys.addEdge("25", "66");
				sys.addEdge("32", "47");
				sys.addEdge("32", "84");
				sys.addEdge("42", "32");
				sys.addEdge("42", "7");
				sys.addEdge("50", "72");
				sys.addEdge("50", "65");
				sys.addEdge("53", "67");
				sys.addEdge("53", "68");
				sys.addEdge("66", "79");
				sys.addEdge("66", "80");
				sys.addEdge("67", "88");
				sys.addEdge("67", "83");
				sys.addEdge("68", "77");
				sys.addEdge("68", "91");
				sys.addEdge("80", "99");
				sys.addEdge("80", "97");
				sys.addEdge("88", "110");
				sys.addEdge("88", "104");
				sys.addEdge("91", "106");
				sys.addEdge("91", "100");

				sys.start();

                return that;
            },

            redraw:function () {
                if (!sys) return;

                ctx.clearRect(0, 0, _canvas.width, _canvas.height);

                var nodeBoxes = [];
                sys.eachNode(function (node, pt) {
                    var label = "" + node._id;
                    var w = ctx.measureText(label).width + 10;
                    pt.x = Math.floor(pt.x);
                    pt.y = Math.floor(pt.y);

                    ctx.fillStyle = node.color;
					ctx.fillRect(pt.x - w / 2, pt.y - 10, w, 20);
                    nodeBoxes[node.name] = [pt.x - w / 2, pt.y - 11, w, 22];

                    ctx.font = "12px Helvetica";
                    ctx.textAlign = "center";
                    ctx.fillStyle = "white";
                    ctx.fillText(label, pt.x, pt.y + 4);
                });

                sys.eachEdge(function (edge, pt1, pt2) {
                    var color = "#555555";
                    var tail = intersect_line_box(pt1, pt2, nodeBoxes[edge.source.name]);
                    var head = intersect_line_box(tail, pt2, nodeBoxes[edge.target.name]);

                    ctx.save();
                    ctx.beginPath();
                    ctx.lineWidth = 1;
                    ctx.strokeStyle = color;
                    ctx.moveTo(tail.x, tail.y);
                    ctx.lineTo(head.x, head.y);
                    ctx.stroke();
                    ctx.restore();

                    // draw an arrowhead if this is a -> style edge
                    if (edge.directed) {
                        ctx.save();
                        // move to the head position of the edge we just drew
                        var wt = 1;
                        var arrowLength = 6 + wt;
                        var arrowWidth = 2 + wt;
                        ctx.fillStyle = color;
                        ctx.translate(head.x, head.y);
                        ctx.rotate(Math.atan2(head.y - tail.y, head.x - tail.x));

                        // delete some of the edge that's already there (so the point isn't hidden)
                        ctx.clearRect(-arrowLength / 2, -wt / 2, arrowLength / 2, wt);

                        // draw the chevron
                        ctx.beginPath();
                        ctx.moveTo(-arrowLength, arrowWidth);
                        ctx.lineTo(0, 0);
                        ctx.lineTo(-arrowLength, -arrowWidth);
                        ctx.lineTo(-arrowLength * 0.8, -0);
                        ctx.closePath();
                        ctx.fill();
                        ctx.restore();
                    }
                });
            }
		};

        var intersect_line_line = function (p1, p2, p3, p4) {
            var denom = ((p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y));
            if (denom === 0) return false; // lines are parallel
            var ua = ((p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x)) / denom;
            var ub = ((p2.x - p1.x) * (p1.y - p3.y) - (p2.y - p1.y) * (p1.x - p3.x)) / denom;

            if (ua < 0 || ua > 1 || ub < 0 || ub > 1) return false;
            return new ArbPoint(p1.x + ua * (p2.x - p1.x), p1.y + ua * (p2.y - p1.y));
        };

        var intersect_line_box = function (p1, p2, boxTuple) {
            var bx = boxTuple[0];
			var by = boxTuple[1];
			var bw = boxTuple[2];
			var bh = boxTuple[3];

            var tl = {x: bx, y: by};
            var tr = {x: bx + bw, y: by};
            var bl = {x: bx, y: by + bh};
            var br = {x: bx + bw, y: by + bh};

            return intersect_line_line(p1, p2, tl, tr) ||
                intersect_line_line(p1, p2, tr, br) ||
                intersect_line_line(p1, p2, br, bl) ||
                intersect_line_line(p1, p2, bl, tl) ||
                false;
        };

        return that.init();
    };


    HalfViz();

})();