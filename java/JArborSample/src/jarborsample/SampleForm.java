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

import arborgvt.ArborNode;
import arborgvt.ArborViewer;
import bslib.common.FramesHelper;
import bslib.common.StringHelper;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionAdapter;
import javax.swing.JFrame;

/**
 *
 * @author Serg V. Zhdanovskih
 */
public class SampleForm extends JFrame
{
    private ArborViewer arborViewer1;

    public SampleForm()
    {
        super();

        this.initializeComponents();

        arborViewer1.setEnergyDebug(true);
        arborViewer1.setNodesDragging(true);
        arborViewer1.doSample();
        arborViewer1.start();
    }

    private void initializeComponents()
    {
        FramesHelper.setClientSize(this, 894, 587);

        this.arborViewer1 = new ArborViewer();
        this.arborViewer1.setBackground(Color.white);
        this.arborViewer1.setPreferredSize(new Dimension(894, 587));

        this.setLayout(new BorderLayout());
        this.add(arborViewer1, BorderLayout.CENTER);
        this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        this.setLocationRelativeTo(null);
        this.setTitle("JArbor sample");

        this.addMouseMotionListener(new MouseMotionAdapter()
        {
            @Override
            public void mouseMoved(MouseEvent e)
            {
                onMouseMove(e);
            }
        });
    }

    private void onMouseMove(MouseEvent e)
    {
        ArborNode resNode = arborViewer1.getNodeByCoord(e.getX(), e.getY());

        String hint = "";
        if (resNode != null) {
            hint = resNode.Sign;
        }

        if (!StringHelper.equals(hint, "")) {
            arborViewer1.setToolTipText(hint);
        } else {
            arborViewer1.setToolTipText(null);
        }
    }
}
