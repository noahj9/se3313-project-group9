
import javax.swing.*;
import java.awt.*;
import components.*;
import java.io.*;
import java.net.*;
import java.awt.event.*;
import java.util.concurrent.TimeUnit;

public class Panel extends JPanel implements CountdownPanel.CountdownListener, Multiplier.MultiplierListener {
    private CountdownPanel countdownPanel;
    private Multiplier multiplier;
    private Player player;
    private Explosion explosion;
    private LeftPanel leftPanel;
    private RightPanel rightPanel;
    private CenterPanel centerPanel;

    public String userId = "1";
    public String roomNumber;

    public int selectedRoomIndex = 0;
    public String roomListStrings[] = {"Room_0", "Room_1", "Room_2", "Room_3", "Room_4"};

    public Panel() {
        // setLayout(new BorderLayout());
        // countdownPanel = new CountdownPanel();
        // countdownPanel.addCountdownListener(this);
        // add(countdownPanel, BorderLayout.NORTH);

        leftPanel = new LeftPanel();
        rightPanel = new RightPanel();
        centerPanel = new CenterPanel();

        JPanel mainPanel = new JPanel(new BorderLayout());
        mainPanel.add(leftPanel, BorderLayout.WEST);
        mainPanel.add(rightPanel, BorderLayout.EAST);
        mainPanel.add(centerPanel, BorderLayout.CENTER);
        add(mainPanel, BorderLayout.SOUTH);

    }

    public void countdownFinished() {
        remove(countdownPanel);
        multiplier = new Multiplier();
        multiplier.addMultiplierListener(this);
        add(multiplier, BorderLayout.NORTH);
        player = new Player();
        add(player, BorderLayout.CENTER);
        revalidate();
        repaint();
    }

    public void multiplierStopped() {
        Timer timer2 = new Timer(150, e -> {
            remove(player);
            explosion = new Explosion();
            add(explosion, BorderLayout.CENTER);
            revalidate();
            repaint();
        });
        timer2.setRepeats(false);
        timer2.start();
        Timer timer = new Timer(1500, e -> {
            remove(explosion);
            remove(multiplier);
            countdownPanel = new CountdownPanel();
            countdownPanel.addCountdownListener(this);
            add(countdownPanel, BorderLayout.NORTH);
            revalidate();
            repaint();
        });
        timer.setRepeats(false);
        timer.start();
    }

    // Display the current selected room
    JLabel selectedRoomLabel = new JLabel(roomListStrings[selectedRoomIndex]);

    private class LeftPanel extends JPanel {
        public LeftPanel() {
            setLayout(new GridBagLayout());
            GridBagConstraints leftConstraints = new GridBagConstraints();
            leftConstraints.gridx = 0;
            leftConstraints.gridy = 0;
            leftConstraints.anchor = GridBagConstraints.LINE_START;
            leftConstraints.insets = new Insets(5, 5, 5, 5);

            JButton newGameButton = new JButton("New Game Room");
            newGameButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    System.out.println("New Game button pressed");
                    try {
                        Socket socket = new Socket("127.0.0.1", 2003);
                        OutputStream outputStream = socket.getOutputStream();

                        // TODO: This will have to be an exact room name based on what's selected
                        String request = "CREATE_ROOM";
                        outputStream.write(request.getBytes());
                        socket.close();
                    } catch (IOException ex) {
                        ex.printStackTrace();
                    }
                }
            });
            add(newGameButton, leftConstraints);

            JButton refreshButton = new JButton("Refresh Room List");
            refreshButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    // TODO: Send "GET_ACTIVE_ROOMS" message to the server
                    // Refresh rooms :)
                    System.out.println("Refresh Room List button pressed");
                    try {
                        Socket socket = new Socket("127.0.0.1", 2003);
                        OutputStream outputStream = socket.getOutputStream();

                        // TODO: This will have to be an exact room name based on what's selected
                        String request = "GET_ACTIVE_ROOMS";
                        outputStream.write(request.getBytes());
                        socket.close();
                    } catch (IOException ex) {
                        ex.printStackTrace();
                    }
                }
            });
            leftConstraints.gridy = 1;
            add(refreshButton, leftConstraints);
        }
    }

    // Inner class for the right panel
    private class RightPanel extends JPanel {
        public RightPanel() {
            setLayout(new GridBagLayout());
            GridBagConstraints rightConstraints = new GridBagConstraints();
            rightConstraints.gridx = 0;
            rightConstraints.gridy = 0;
            rightConstraints.anchor = GridBagConstraints.LINE_END;
            rightConstraints.insets = new Insets(5, 5, 5, 5);

            JButton upButton = new JButton("^");
            upButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    selectedRoomIndex = (selectedRoomIndex - 1 + roomListStrings.length) % roomListStrings.length;
                    selectedRoomLabel.setText(roomListStrings[selectedRoomIndex]);
                }
            });
            add(upButton, rightConstraints);

            JButton downButton = new JButton("v");
            downButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    selectedRoomIndex = (selectedRoomIndex + 1) % roomListStrings.length;
                    selectedRoomLabel.setText(roomListStrings[selectedRoomIndex]);
                }
            });
            rightConstraints.gridy = 1;
            add(downButton, rightConstraints);
        }
    }

    private class CenterPanel extends JPanel {
    
        public CenterPanel() {
            setLayout(new GridBagLayout());
            GridBagConstraints centerConstraints = new GridBagConstraints();
            centerConstraints.gridx = 0;
            centerConstraints.gridy = 0;
            centerConstraints.anchor = GridBagConstraints.CENTER;
            centerConstraints.insets = new Insets(5, 5, 5, 5);
    
            add(selectedRoomLabel, centerConstraints);
    
            // Constraints for the room label
            GridBagConstraints roomLabelConstraints = new GridBagConstraints();
            roomLabelConstraints.gridx = 0;
            roomLabelConstraints.gridy = 0;
            roomLabelConstraints.anchor = GridBagConstraints.CENTER;
            roomLabelConstraints.insets = new Insets(5, 5, 5, 5);
            add(selectedRoomLabel, roomLabelConstraints);
    
            // Constraints for the select room button
            GridBagConstraints selectButtonConstraints = new GridBagConstraints();
            selectButtonConstraints.gridx = 0;
            selectButtonConstraints.gridy = 1;
            selectButtonConstraints.anchor = GridBagConstraints.CENTER;
            selectButtonConstraints.insets = new Insets(5, 5, 5, 5);
            JButton selectRoomButton = new JButton("Select Room");
            selectRoomButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    // Send the selected room to the server
                    try {
                        Socket socket = new Socket("127.0.0.1", 2003);
                        OutputStream outputStream = socket.getOutputStream();
    
                        // Send the selected room
                        String request = "JOIN_ROOM " + roomListStrings[selectedRoomIndex] + " " + userId;
                        
                        outputStream.write(request.getBytes());
                        socket.close();
                    } catch (IOException ex) {
                        ex.printStackTrace();
                    }
                }
            });
            add(selectRoomButton, selectButtonConstraints);
        }
    }
    
    // bet - up and down arrow with bet balance
    // cash out
    // leave room

    public static void initializeUser() {
        // Logic for creating user --> join the server through a socket connection
        // Should receive a "userId" (string) from the server. Store this in a global variable
    }

    public static void main(String[] args) {
        // User handling to add user -> create user
        SwingUtilities.invokeLater(() -> {
            JFrame frame = new JFrame("Loop of full animation");
            frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
            frame.setSize(800, 600);

            Panel mainPanel = new Panel();
            frame.add(mainPanel);
            frame.setVisible(true);
        });
    }
}
