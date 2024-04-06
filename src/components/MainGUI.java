package components;
import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.*;
import java.net.*;
import java.io.*;


public class MainGUI {

    public class JoinRoomListener  implements ActionListener
	{
        public void actionPerformed(ActionEvent e) {
            try {
                // Connect to C++ backend server
                Socket socket = new Socket("localhost", 8080);
                OutputStream outputStream = socket.getOutputStream();
                
                // Send request to join room
                String request = "JOIN_ROOM Room1";
                outputStream.write(request.getBytes());
                
                // Close connection
                socket.close();
            } catch (IOException ex) {
                ex.printStackTrace();
            }
        }
	}
    public static void main(String[] args) {
        JFrame frame = new JFrame("Java GUI");
        JButton button = new JButton("Join Room");
        button.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                try {
                    // Connect to C++ backend server
                    Socket socket = new Socket("localhost", 8080);
                    OutputStream outputStream = socket.getOutputStream();
                    
                    // Send request to join room
                    String request = "JOIN_ROOM Room1";
                    outputStream.write(request.getBytes());
                    
                    // Close connection
                    socket.close();
                } catch (IOException ex) {
                    ex.printStackTrace();
                }
            }
        });
        frame.getContentPane().add(button);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setSize(300, 200);
        frame.setVisible(true);
    }
}