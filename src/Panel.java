
import javax.swing.*;
import javax.swing.Timer;

import java.awt.*;
import components.*;
import java.io.*;
import java.net.*;
import java.awt.event.*;
import java.util.*;
import java.awt.List;
import java.util.concurrent.TimeUnit;

public class Panel extends JPanel implements CountdownPanel.CountdownListener, Multiplier.MultiplierListener {
    private CountdownPanel countdownPanel;
    private Multiplier multiplier;
    private Player player;
    private Explosion explosion;
    public LeftPanel leftPanel;
    public RightPanel rightPanel;
    public CenterPanel centerPanel;
    public GameRoomCenterPanel gRoomCentralPanel;
    public GameRoomLeftPanel gRoomLeftPanel;
    public CashoutCenterPanel cashoutCenterPanel;
    public RoomsListCenterPanel roomListPanel;
    public JPanel mainPanel;

    public String userId = "";
    public String roomNumber = "";

    public int selectedRoomIndex = 0;
    public String roomListStrings[] = {"Room_0", "Room_1", "Room_2", "Room_3", "Room_4"};

    public Panel() {
        setLayout(new BorderLayout());
        // countdownPanel = new CountdownPanel();
        // countdownPanel.addCountdownListener(this);
        // add(countdownPanel, BorderLayout.NORTH);
      
        if(roomNumber.length()==0){
            leftPanel = new LeftPanel();
            rightPanel = new RightPanel();
            centerPanel = new CenterPanel();
            roomListPanel = new RoomsListCenterPanel();
          
            mainPanel = new JPanel(new BorderLayout());
            mainPanel.add(roomListPanel, BorderLayout.NORTH);
            mainPanel.add(leftPanel, BorderLayout.WEST);
            mainPanel.add(rightPanel, BorderLayout.EAST);
            mainPanel.add(centerPanel, BorderLayout.CENTER);

            roomListPanel.highlightRoom(selectedRoomIndex);

            add(mainPanel, BorderLayout.SOUTH);
        }else{
            gRoomLeftPanel = new GameRoomLeftPanel();
            gRoomCentralPanel = new GameRoomCenterPanel();

            mainPanel = new JPanel(new BorderLayout());
            mainPanel.add(gRoomLeftPanel, BorderLayout.WEST);
            mainPanel.add(gRoomCentralPanel, BorderLayout.CENTER);
            add(mainPanel, BorderLayout.SOUTH);
        }
    }

    private class GameRoomLeftPanel extends JPanel {
        public GameRoomLeftPanel() {
            setLayout(new GridBagLayout());
            GridBagConstraints leftConstraints = new GridBagConstraints();
            leftConstraints.gridx = 0;
            leftConstraints.gridy = 0;
            leftConstraints.anchor = GridBagConstraints.LINE_START;
            JButton newGameButton = new JButton("Leave Room");
            newGameButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    System.out.println("Leave Room");
                    leaveRoom();
                }
            });
            add(newGameButton, leftConstraints);
        }
    }
    
    private class GameRoomCenterPanel extends JPanel {
        public GameRoomCenterPanel() {
            setLayout(new GridBagLayout());
            GridBagConstraints centerConstraints = new GridBagConstraints();
            centerConstraints.gridx = 0;
            centerConstraints.gridy = 0;
            centerConstraints.anchor = GridBagConstraints.CENTER;
            JButton upButton = new JButton("^");
            upButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    System.out.println("Up button pressed");
                }
            });
            add(upButton, centerConstraints);
    
            
            JButton placeBetButton = new JButton("Bet");
            placeBetButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    System.out.println("Bet Placed");
                    removeBetButtons();

                }
            });
            centerConstraints.gridy = 1;
            add(placeBetButton, centerConstraints);

            JButton downButton = new JButton("v");
            downButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    System.out.println("Down button pressed");
                }
            });
            centerConstraints.gridy = 2;
            add(downButton, centerConstraints);
        }
    }

    private class CashoutCenterPanel extends JPanel {
        public CashoutCenterPanel() {
            setLayout(new GridBagLayout());
            GridBagConstraints centerConstraints = new GridBagConstraints();
            centerConstraints.gridx = 0;
            centerConstraints.gridy = 0;
            centerConstraints.anchor = GridBagConstraints.CENTER;
            centerConstraints.insets = new Insets(5, 5, 5, 5);
    
            JButton selectRoomButton = new JButton("CashOut");
            selectRoomButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    System.out.println("Cashout");
                    removeCashOut();
                }
            });
            add(selectRoomButton, centerConstraints);
        }
    }

    private class RoomsListCenterPanel extends JPanel {
        private JLabel[] roomLabels;
    
        public RoomsListCenterPanel() {
            setLayout(new GridLayout(roomListStrings.length, 1));
            roomLabels = new JLabel[roomListStrings.length];
    
            for (int i = 0; i < roomListStrings.length; i++) {
                roomLabels[i] = new JLabel(roomListStrings[i]);
                roomLabels[i].setHorizontalAlignment(SwingConstants.CENTER);
                add(roomLabels[i]);
            }
        }
    
        // Method to highlight the selected room
        public void highlightRoom(int selectedIndex) {
            for (int i = 0; i < roomLabels.length; i++) {
                if (i == selectedIndex) {
                    roomLabels[i].setBackground(Color.YELLOW); // Highlight color
                    roomLabels[i].setOpaque(true);
                } else {
                    roomLabels[i].setBackground(null); // Reset color
                }
            }
        }
    }       

    public void removeBetButtons(){
        System.out.println("Remove Bet called");
        mainPanel.remove(gRoomCentralPanel);
        mainPanel.remove(gRoomLeftPanel);
        cashoutCenterPanel = new CashoutCenterPanel();
        mainPanel.add(cashoutCenterPanel, BorderLayout.CENTER);
        revalidate();
        repaint();

    }
    public void removeCashOut(){
        System.out.println("Cashout called");
        mainPanel.remove(cashoutCenterPanel);
        gRoomLeftPanel = new GameRoomLeftPanel();
        gRoomCentralPanel = new GameRoomCenterPanel();
        mainPanel.add(gRoomLeftPanel, BorderLayout.WEST);
        mainPanel.add(gRoomCentralPanel, BorderLayout.CENTER);
        add(mainPanel, BorderLayout.SOUTH);
        revalidate();
        repaint();
    }
    public void leaveRoom(){
        mainPanel.remove(gRoomCentralPanel);
        mainPanel.remove(gRoomLeftPanel);
        leftPanel = new LeftPanel();
        rightPanel = new RightPanel();
        centerPanel = new CenterPanel();
        roomListPanel = new RoomsListCenterPanel();

        mainPanel.add(roomListPanel, BorderLayout.NORTH);
        mainPanel.add(leftPanel, BorderLayout.WEST);
        mainPanel.add(rightPanel, BorderLayout.EAST);
        mainPanel.add(centerPanel, BorderLayout.CENTER);

        roomListPanel.highlightRoom(selectedRoomIndex);

        add(mainPanel, BorderLayout.SOUTH);
        revalidate();
        repaint();

    }
    public void getGameRoomPanel(){
        mainPanel.remove(leftPanel);
        mainPanel.remove(rightPanel);
        mainPanel.remove(centerPanel);
        mainPanel.remove(roomListPanel);
        gRoomLeftPanel = new GameRoomLeftPanel();
        gRoomCentralPanel = new GameRoomCenterPanel();
        mainPanel.add(gRoomLeftPanel, BorderLayout.WEST);
        mainPanel.add(gRoomCentralPanel, BorderLayout.CENTER);
        add(mainPanel, BorderLayout.SOUTH);
        revalidate();
        repaint();
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

    public class RoomListPanel extends JPanel {
        private String[] roomListStrings;

        public RoomListPanel(String[] roomListStrings) {
            this.roomListStrings = roomListStrings;
            setupUI();
        }

        private void setupUI() {
            setLayout(new GridLayout(roomListStrings.length + 1, 1));

            // Add labels for each room in the room list
            for (String room : roomListStrings) {
                JLabel roomLabel = new JLabel(room);
                roomLabel.setHorizontalAlignment(SwingConstants.CENTER);
                add(roomLabel);
            }

            // Add a button for room selection
            JButton selectRoomButton = new JButton("Select Room");
            selectRoomButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    // Handle room selection here
                    System.out.println("Selected room: " + roomListStrings[selectedRoomIndex]);
                }
            });
            add(selectRoomButton);
        }
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
                    selectedRoomIndex = (selectedRoomIndex - 1 + roomListStrings.length) % roomListStrings.length;
                    selectedRoomLabel.setText(roomListStrings[selectedRoomIndex]);
                    roomListPanel.highlightRoom(selectedRoomIndex);
                }
            });
            add(upButton, rightConstraints);

            JButton downButton = new JButton("v");
            downButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    selectedRoomIndex = (selectedRoomIndex + 1) % roomListStrings.length;
                    selectedRoomLabel.setText(roomListStrings[selectedRoomIndex]);
                    roomListPanel.highlightRoom(selectedRoomIndex);
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
                    roomNumber = roomListStrings[selectedRoomIndex];
                    System.out.println("Selected room: " + roomNumber);
                    getGameRoomPanel();
                    /*try {
                        Socket socket = new Socket("127.0.0.1", 2003);
                        OutputStream outputStream = socket.getOutputStream();
    
                        // Send the selected room
                        String request = "JOIN_ROOM " + roomListStrings[selectedRoomIndex] + " " + userId;

                        roomNumber = roomListStrings[selectedRoomIndex];
                        
                        // outputStream.write(request.getBytes());
                        socket.close();
                    } catch (IOException ex) {
                        ex.printStackTrace();
                    }*/
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
