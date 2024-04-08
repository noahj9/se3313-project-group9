
import javax.swing.*;
import java.awt.*;
import components.*;
import java.io.*;
import java.net.*;
import java.awt.event.*;
import java.util.*;
import java.awt.List;

public class Panel extends JPanel implements CountdownPanel.CountdownListener, Multiplier.MultiplierListener {
    private CountdownPanel countdownPanel;
    private Multiplier multiplier;
    private Player player;
    private Explosion explosion;
    private LeftPanel leftPanel;
    private RightPanel rightPanel;
    private CenterPanel centerPanel;

    public String userId;
    public String roomNumber;

    public Panel() {
        setLayout(new BorderLayout());
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
                        // open socket connection
                        // TODO: change localhost to the AWS server IP address
                        Socket socket = new Socket("127.0.0.1", 2003);

                        // create output stream
                        OutputStream outputStream = socket.getOutputStream();
                        PrintWriter out = new PrintWriter(outputStream, true);

                        // send the message
                        out.println("CREATE_ROOM");

                        // close the connection
                        // TODO: @noah make a GLOBAL socket for the client, so we can always use the same socket
                        socket.close();

                        System.out.println("New Game button pressed and CREATE_ROOM message sent");
                    } catch (IOException ex) {
                        // handle exception if error with socket
                        ex.printStackTrace();
                    }
                }
            });
            add(newGameButton, leftConstraints);

            JButton refreshButton = new JButton("Refresh Room List");
            refreshButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    System.out.println("Refresh Room List button pressed");
                    try {
                        Socket socket = new Socket("127.0.0.1", 2003);
                        OutputStream outputStream = socket.getOutputStream();

                        String request = "GET_ACTIVE_ROOMS";
                        outputStream.write(request.getBytes());
            
                        BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                        ArrayList<String> roomList = new ArrayList<>();
                        String inputLine;
                        // inputLine = Room Name: Room_0, Players: 1Room Name: Room_1, Players: 1Room Name: Room_2, Players: 1
                        while ((inputLine = in.readLine()) != null && !inputLine.trim().isEmpty()) {
                            roomList.add(inputLine); // Add each line (room) to the list
                            System.out.println(inputLine);
                        }
                        socket.close(); // Close connection

                        // Now, roomList contains all the rooms, and you can log it
                        System.out.println("Received list of active rooms:");
                        for (String room : roomList) {
                            System.out.println(room);
                        }
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
                    System.out.println("Up button pressed");
                }
            });
            add(upButton, rightConstraints);

            JButton downButton = new JButton("v");
            downButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    System.out.println("Down button pressed");
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

            JButton selectRoomButton = new JButton("Select Room");
            selectRoomButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    // TODO: @noah Same socket issue as above

                    try {
                        Socket socket = new Socket("127.0.0.1", 2003);
                        OutputStream outputStream = socket.getOutputStream();

                        // TODO: This will have to be an exact room name based on what's selected
                        String request = "JOIN_ROOM Room_0 userID";
                        outputStream.write(request.getBytes());
                        socket.close();
                    } catch (IOException ex) {
                        ex.printStackTrace();
                    }
                }
            });
            add(selectRoomButton, centerConstraints);
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
