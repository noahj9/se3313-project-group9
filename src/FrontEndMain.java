import java.awt.*;
import java.awt.event.*;
import java.net.*;
import java.io.OutputStream;
import java.io.InputStream;
import java.lang.Thread;

import javax.swing.Action;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;

//import statements

public class FrontEndMain extends JFrame {

	JLabel jlbHelloWorld;
	private OutputStream socketOutput;
	private InputStream socketInput;
	private Socket theSocket;
	public void SendString(String s)
	{
		try
		{
			socketOutput.write(s.getBytes());
			System.out.println("Sent:"+s);
		}
		catch(Exception e)
		{
			System.out.println(e.toString());
		}
	}

	public class UpButtonListener  implements ActionListener
	{
		public void actionPerformed(ActionEvent e)
		{
			SendString("Up");
		}
	}
	public class RightButtonListener implements ActionListener
	{
		public void actionPerformed(ActionEvent e)
		{
			SendString("Right");
		}
	}
	public class DownButtonListener implements ActionListener
	{
		public void actionPerformed(ActionEvent e)
		{
			SendString("Down");
		}
	}
	public class LeftButtonListener implements ActionListener
	{
		public void actionPerformed(ActionEvent e)
		{
			SendString("left");
		}
	}
	public class FireButtonListener implements ActionListener
	{
		public void actionPerformed(ActionEvent e)
		{
			System.out.println("I have no effect");
		}
	}
	public class ConnectionThread extends Thread
	{
		public ConnectionThread()
		{
			
		}
		int MakeInt(byte hi, byte lo)
		{
			int h = (int)hi;
			h = h & 0x00FF;
			h = h << 8;
			int l = (int)lo;
			l = l & 0x00FF;
			return h | l;
		}
		public void run()
		{
			try
			{
				byte[] readBuffer = new byte[256];
				while(true)
				{
					socketInput.read(readBuffer);
					String decoded = new String(readBuffer, "UTF-8");
					if (decoded.charAt(0)=='Q')
					{
						theSocket.close();
						System.out.println("Received disconnect");
						theField.setDisconnect();
						return;

					}
					else if (decoded.charAt(0)=='W')
					{
						System.out.println("Wait for another player");
						theField.setWaiting();
					}
					else if (decoded.charAt(0)=='A')
					{
						Point ball = new Point(MakeInt(readBuffer[2],readBuffer[3]),MakeInt(readBuffer[4],readBuffer[5]));
						Point p1 = new Point(480,MakeInt(readBuffer[7],readBuffer[8]));
						Point p2 = new Point(5,MakeInt(readBuffer[10],readBuffer[11]));
						String score = String.format("%d - %d",readBuffer[6],readBuffer[9]);
						theField.setState(decoded.charAt(1),p1, p2, ball,score);
					}						
				}				
			}
			catch (Exception e)
			{
				System.out.println(e.toString());
			}			
		}
	}
	ConnectionThread theConnection;
	public class ConnectButtonListener implements ActionListener
	{
		public void actionPerformed(ActionEvent e)
		{
			String one = new String("hello");
			String two = new String("world");
			try
			{
				//theSocket = new Socket(InetAddress.getByName("129.100.227.119"),2000);
				theSocket = new Socket(InetAddress.getByName("127.0.0.1"), 2000);
				socketOutput = theSocket.getOutputStream();
				socketInput = theSocket.getInputStream();
				theConnection = new ConnectionThread();
				theConnection.start();
				System.out.println("connect");
			}
			catch(Exception ex)
			{
				System.out.println(ex.toString());
			}
		}
	}
	public class DisconnectButtonListener implements ActionListener
	{
		public void actionPerformed(ActionEvent e)
		{
			try
			{
				theSocket.close();
				theField.setDisconnect();
			}
			catch(Exception ex)
			{
				System.out.println(ex.toString());
			}
			System.out.println("disconnect");
		}
	}
	public static int DISCONNECT = 0;
	public static int WAIT = 1;
	public static int PLAYING = 2;
	
	public class PlayingField extends Canvas
	{
		Dimension theDimension;
		Dimension minDim;
		Point player1;
		Point player2;
		Point ball;
		int state;
		int playerId;
		String score;
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
	public static void main(String args[]) 
	{
		new FrontEndMain();
	}
	PlayingField theField;
	FrontEndMain()
	{
        jlbHelloWorld = new JLabel("Hello World");
        JButton upButton = new JButton("^");
        upButton.addActionListener(new UpButtonListener());
        upButton.setSize(100,20);
        JButton rightButton = new JButton(">");
        rightButton.addActionListener(new RightButtonListener());
        rightButton.setSize(100,20);
        JButton downButton = new JButton("v");
        downButton.addActionListener(new DownButtonListener());
        downButton.setSize(100,20);
        JButton leftButton = new JButton("<");
        leftButton.addActionListener(new LeftButtonListener());
        leftButton.setSize(100,20);
        JButton fireButton = new JButton("O");
        fireButton.addActionListener(new FireButtonListener());
        fireButton.setSize(100,20);
        
        JPanel directionPanel = new JPanel(new GridBagLayout());
        GridBagConstraints c = new GridBagConstraints();
        c.fill = GridBagConstraints.HORIZONTAL;
        c.gridx = 1; c.gridy = 0;        
        directionPanel.add(upButton,c);
        c.gridx = 0; c.gridy = 1;
        directionPanel.add(leftButton,c);
        c.gridx = 1; c.gridy = 2;
        directionPanel.add(downButton,c);
        c.gridx = 2; c.gridy = 1;        
        directionPanel.add(rightButton,c);
        c.gridx = 1; c.gridy = 1;
        directionPanel.add(fireButton,c);

        JPanel connectionPanel = new JPanel(new GridBagLayout());
        JButton connectButton = new JButton("Connect");
        connectButton.addActionListener(new ConnectButtonListener());
        c.gridx = 0; c.gridy = 0;
        connectionPanel.add(connectButton,c);
        JButton disconnectButton = new JButton("Disconnect");
        disconnectButton.addActionListener(new DisconnectButtonListener());
        c.gridx = 1; c.gridy = 0;
        connectionPanel.add(disconnectButton,c);
        
        Rectangle theBounds = getBounds();
        Dimension d = new Dimension();
        d.width = 500;
        d.height = 450;
        theField = new PlayingField(d);
        
        /*setLayout(new GridLayout(2,2));
        add(theField);
        add(connectionPanel);
        add(directionPanel);*/
        setLayout(new GridBagLayout());
        c.gridx = 0; c.gridy = 0;
        c.gridheight = 9;
        c.gridwidth = 10;
        c.weightx = 1.0;c.weighty=1.0;
        c.fill = GridBagConstraints.BOTH;
        add(theField,c);
        c.gridx = 0; c.gridy = 9;
        c.gridheight = 1;
        c.gridwidth = 2;
        c.fill = GridBagConstraints.NONE;
        add(connectionPanel,c);
        c.gridx = 7;c.gridy = 9;
        c.gridheight = 1;
        c.gridwidth = 2;
        c.fill = GridBagConstraints.NONE;
        add(directionPanel,c);
        this.setSize(500,500);
        this.setTitle("Network Game Console");

        setVisible(true);
	}
}

