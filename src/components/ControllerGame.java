package components;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.net.*;
import java.io.*;
    
public class ControllerGame extends JFrame {

    public static void main(String args[]) {
        SwingUtilities.invokeLater(() -> {
            new ControllerGame();
        });
    }

    ControllerGame() {
        JPanel leftPanel = new JPanel(new GridBagLayout());
        GridBagConstraints leftConstraints = new GridBagConstraints();
        leftConstraints.gridx = 0;
        leftConstraints.gridy = 0;
        leftConstraints.anchor = GridBagConstraints.LINE_START;
        leftConstraints.insets = new Insets(5, 5, 5, 5);

        JButton newGameButton = new JButton("New Game");
        newGameButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                System.out.println("New Game button pressed");
            }
        });
        leftPanel.add(newGameButton, leftConstraints);

        JButton refreshButton = new JButton("Refresh Room List");
        refreshButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                System.out.println("Refresh Room List button pressed");
            }
        });
        leftConstraints.gridy = 1;
        leftPanel.add(refreshButton, leftConstraints);

        JPanel rightPanel = new JPanel(new GridBagLayout());
        GridBagConstraints rightConstraints = new GridBagConstraints();
        rightConstraints.gridx = 0;
        rightConstraints.gridy = 0;
        rightConstraints.anchor = GridBagConstraints.LINE_END;
        rightConstraints.insets = new Insets(5, 5, 5, 5);

        JButton upButton = new JButton("^");
        upButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                System.out.println("Up button pressed");
            }
        });
        rightPanel.add(upButton, rightConstraints);

        JButton downButton = new JButton("v");
        downButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                System.out.println("Down button pressed");
            }
        });
        rightConstraints.gridy = 1;
        rightPanel.add(downButton, rightConstraints);

        JPanel centerPanel = new JPanel(new GridBagLayout());
        GridBagConstraints centerConstraints = new GridBagConstraints();
        centerConstraints.gridx = 0;
        centerConstraints.gridy = 0;
        centerConstraints.anchor = GridBagConstraints.CENTER;
        centerConstraints.insets = new Insets(5, 5, 5, 5);

        JButton selectRoomButton = new JButton("Select Room");
        selectRoomButton.addActionListener(new ActionListener() {
            @Override
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
        centerPanel.add(selectRoomButton, centerConstraints);

        JPanel mainPanel = new JPanel(new BorderLayout());
        mainPanel.add(leftPanel, BorderLayout.WEST);
        mainPanel.add(rightPanel, BorderLayout.EAST);
        mainPanel.add(centerPanel, BorderLayout.CENTER);

        this.getContentPane().add(mainPanel);
        this.setTitle("Network Game Console");
        this.setSize(500, 200);
        this.setVisible(true);
        this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    }
}
