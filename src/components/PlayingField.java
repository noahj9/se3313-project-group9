package components;
import components.*;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import components.PlayingField;
    

public class PlayingField extends Canvas{
        Dimension theDimension;
		Dimension minDim;
		Point player1;
		Point player2;
		Point ball;
		int state;
		int playerId;
		String score;
		public static int DISCONNECT = 0;
		public static int WAIT = 1;
		public static int PLAYING = 2;
		public PlayingField (Dimension d)
		{
			theDimension = d;
			minDim = new Dimension();
			minDim.width = d.width/2;
			minDim.height = d.height/2;
			state = DISCONNECT;
		}
		public Dimension getPreferredSize()
		{
			return theDimension;
		}
		public Dimension getMinimumSize()
		{
			return minDim;
		}
		public void paint(Graphics g)
		{
			g.setColor(Color.black);
			Rectangle bounds= getBounds();
			g.fillRect(bounds.x,bounds.y,bounds.width,bounds.height);
			//System.out.println(bounds.toString());
			
			if (state == DISCONNECT)
			{
				int fontSize = 20;
			    g.setFont(new Font("TimesRoman", Font.PLAIN, fontSize));			     
			    g.setColor(Color.white);			    
			    g.drawString("Not connected to host", bounds.width/3, bounds.height/2);					
			}
			else if (state == WAIT)
			{
				int fontSize = 20;
			    g.setFont(new Font("TimesRoman", Font.PLAIN, fontSize));
			    g.setColor(Color.white);
			    g.drawString("Waiting for an opponent", bounds.width/3, bounds.height/2);					
			}
			else if (state == PLAYING)
			{
				int fontSize = 20;
			    g.setFont(new Font("TimesRoman", Font.PLAIN, fontSize));
			    g.setColor(Color.white);
			    if (playerId == 1)
			    	g.drawString("Player 1", bounds.width/3, bounds.height/2);
			    else
			    	g.drawString("Player 2", bounds.width/3, bounds.height/2);
			    int r = 20;
			    g.fillOval(ball.x-r/2,ball.y-r/2,20,20);
			    g.fillRect(player1.x-5,player1.y-304/10,10,304/5);
			    g.fillRect(player2.x-5,player2.y-304/10,10,304/5);
			    g.drawString(score, bounds.width/2, 20);
			}
		}
		public void Update()
		{
			SwingUtilities.invokeLater(new Runnable(){
				public void run()
				{
					repaint();
				}
			});
		}
		public void setWaiting()
		{
			state = WAIT;
			Update();
			
		}
		public void setDisconnect()
		{
			state = DISCONNECT;
			Update();
		}
		public void setState(int id,Point p1, Point p2, Point b,String s)
		{
			playerId = id;
			player1 = p1;
			player2 = p2;
			ball = b;
			state = PLAYING;
			score = s;
			Update();			
		}
	}	