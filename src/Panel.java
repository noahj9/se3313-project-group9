
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
import javax.swing.Timer;

public class Panel extends JPanel implements CountdownPanel.CountdownListener, Multiplier.MultiplierListener {
    public CountdownPanel countdownPanel;
    public Multiplier multiplier;
    public Player player;
    public Explosion explosion;
    public LeftPanel leftPanel;
    public RightPanel rightPanel;
    public CenterPanel centerPanel;
    public GameRoomCenterPanel gRoomCentralPanel;
    public GameRoomLeftPanel gRoomLeftPanel;
    public GameRoomRightPanel gRoomRightPanel;
    public CashoutCenterPanel cashoutCenterPanel;
    public RoomsListCenterPanel roomListPanel;
    public JPanel mainPanel;
    public JPanel gamePanel;
    public JLabel statusLabel;
    public boolean gameInitialized = false;
    public boolean countdownFunctionCalled = false;
    public boolean multiplierCalled = false;
    public boolean gameStarted;
    public boolean userJoinedRound;
    public boolean cashoutPressed = false;
    private Thread listenerThread;

    public static String userId = "";
    public String roomNumber = "";
    public int selectedRoomIndex = 0;
    public String roomListStrings[] = {};
    public static String SERVER_ADDRESS = "3.88.12.145";
    public static int SERVER_PORT = 2003;

    public int betAmount = 0;
    public double cashAmount = 10;
    public boolean gameInProgress = false;

    public Panel() {
        setLayout(new BorderLayout());
        statusLabel = new JLabel("Select a room and user");
        add(statusLabel, BorderLayout.PAGE_START); // Add the label to the panel
        gameStarted = false;
        // Initialize roomListStrings with at least one value to avoid ArrayIndexOutOfBoundsException
        roomListStrings = new String[] {"No rooms available"};
        gamePanel = new JPanel();
        gamePanel.setLayout(new BoxLayout(gamePanel, BoxLayout.X_AXIS));
        player = new Player();
        gamePanel.add(player);
        multiplier = new Multiplier();
        multiplier.setIsStopped(true);
        userJoinedRound = false;
        gameInProgress=false;
        cashoutPressed = false;
        countdownPanel = new CountdownPanel();
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
            if (roomListStrings.length > 0 && !roomListStrings[0].equals("No rooms available")) {
                roomListPanel.highlightRoom(selectedRoomIndex);
            }

            add(mainPanel, BorderLayout.SOUTH);
            add(gamePanel, BorderLayout.CENTER);
            validate();
            repaint();
        }else{
            gRoomLeftPanel = new GameRoomLeftPanel();
            gRoomCentralPanel = new GameRoomCenterPanel();
            gRoomRightPanel = new GameRoomRightPanel();

            mainPanel = new JPanel(new BorderLayout());
            mainPanel.add(gRoomLeftPanel, BorderLayout.WEST);
            mainPanel.add(gRoomCentralPanel, BorderLayout.CENTER);
            mainPanel.add(gRoomRightPanel, BorderLayout.EAST);
            add(mainPanel, BorderLayout.SOUTH);
            add(gamePanel, BorderLayout.CENTER);
            validate();
            repaint();
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
                    updateStatus();
                    try {
                        Socket socket = new Socket("127.0.0.1", 2003);
                        OutputStream outputStream = socket.getOutputStream();

                        String request = "LEAVE_ROOM";
                        outputStream.write(request.getBytes());
                        listenerThread.interrupt();
                        socket.close();
                    } catch (IOException ex) {
                        ex.printStackTrace();
                    }
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

            // Up button
            JButton upButton = new JButton("^");
            upButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    // Increase bet amount
                    if (betAmount + 1 <= cashAmount) {
                        betAmount += 1;
                        updateBetAmount(betAmount);
                    }
                }
            });
            add(upButton, centerConstraints);

            // Bet button
            JButton placeBetButton = new JButton("Bet");
            placeBetButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    if (!gameInProgress && gameStarted) {
                        System.out.println("Bet Placed");
                        
                        try {
                            Socket socket = new Socket(SERVER_ADDRESS, SERVER_PORT);
                            OutputStream outputStream = socket.getOutputStream();
    
                            String request = "BET " + roomNumber + " " + userId + " " + betAmount;
                            outputStream.write(request.getBytes());
    
                            socket.close(); // Close connection
                        } catch (IOException ex) {
                            ex.printStackTrace();
                        }
    
                        removeBetButtons();
                    }
                }
            });
            centerConstraints.gridy = 1;
            add(placeBetButton, centerConstraints);

            // Down button
            JButton downButton = new JButton("v");
            downButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    // Decrease bet amount
                    if (betAmount > 1) {
                        betAmount -= 1;
                        updateBetAmount(betAmount);
                    }
                }
            });
            centerConstraints.gridy = 2;
            add(downButton, centerConstraints);
        }
    }

    private class GameRoomRightPanel extends JPanel {
        private JLabel cashTextLabel;
        private JLabel cashLabel;
        private JLabel betTextLabel;
        private JLabel betLabel;
    
        public GameRoomRightPanel() {
            setLayout(new GridLayout(2, 1));
    
            // Cash label
            cashTextLabel = new JLabel("Cash Amount:");
            cashLabel = new JLabel("$" + cashAmount);
            add(cashTextLabel);
            add(cashLabel);
    
            // Bet label
            betTextLabel = new JLabel("Bet Amount:");
            betLabel = new JLabel("$" + betAmount);
            add(betTextLabel);
            add(betLabel);
        }
    
        // Method to update the bet label
        public void updateBetLabel(int newBetAmount) {
            betAmount = newBetAmount;
            betLabel.setText("$" + betAmount);
        }

        public void updateCashLabel(double newCashAmount) {
            cashAmount = newCashAmount;
            cashLabel.setText("$" + cashAmount);
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

                    try {
                        Socket socket = new Socket(SERVER_ADDRESS, SERVER_PORT);
                        OutputStream outputStream = socket.getOutputStream();

                        String request = "CASHOUT " + roomNumber + " " + userId + " " + multiplier.getMultiplier();
                        outputStream.write(request.getBytes());

                        socket.close(); // Close connection
                    } catch (IOException ex) {
                        ex.printStackTrace();
                    }
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

    public void updateStatus() {
        String statusText = "Room: " + (roomNumber.isEmpty() ? "None" : roomNumber) +
                            ", User ID: " + (userId.isEmpty() ? "Not set" : userId);
        statusLabel.setText(statusText);
    }

    public void updateBetAmount(int newBetAmount) {
        if (newBetAmount >= 0) {
            betAmount = newBetAmount;
            gRoomRightPanel.updateBetLabel(betAmount);
        } else {
            // Display a message or handle the situation when the bet amount goes below 0
            System.out.println("Invalid bet amount");
        }
    }

    public void updateCashAmount(double newCashAmount) {
        if (newCashAmount >= 0) {
            cashAmount = newCashAmount;
            gRoomRightPanel.updateCashLabel(cashAmount);
        } else {
            // Display a message or handle the situation when the bet amount goes below 0
            System.out.println("Invalid bet amount");
        }
    }

    public void removeBetButtons(){
        System.out.println("Remove Bet Buttons called");
        userJoinedRound = true;
        mainPanel.remove(gRoomRightPanel);
        mainPanel.remove(gRoomCentralPanel);
        mainPanel.remove(gRoomLeftPanel);
        cashoutCenterPanel = new CashoutCenterPanel();
        mainPanel.add(cashoutCenterPanel, BorderLayout.CENTER);
        revalidate();
        repaint();
    }
    public void removeBetButtonsNoCashout(){
        System.out.println("Remove Bet Buttons called no cashout");
        mainPanel.remove(gRoomRightPanel);
        mainPanel.remove(gRoomCentralPanel);
        mainPanel.remove(gRoomLeftPanel);
        revalidate();
        repaint();
    }
    public void removeCashOut(){
        System.out.println("Cashout called");
        mainPanel.remove(cashoutCenterPanel);
        cashoutPressed = true;
        // gRoomLeftPanel = new GameRoomLeftPanel();
        // gRoomCentralPanel = new GameRoomCenterPanel();
        // gRoomRightPanel = new GameRoomRightPanel();
        
        // mainPanel.add(gRoomLeftPanel, BorderLayout.WEST);
        // mainPanel.add(gRoomCentralPanel, BorderLayout.CENTER);
        // mainPanel.add(gRoomRightPanel, BorderLayout.EAST);

        add(mainPanel, BorderLayout.SOUTH);
        revalidate();
        repaint();
    }
    public void leaveRoom(){
        roomNumber = "";

        mainPanel.remove(gRoomCentralPanel);
        mainPanel.remove(gRoomLeftPanel);
        mainPanel.remove(gRoomRightPanel);
        leftPanel = new LeftPanel();
        rightPanel = new RightPanel();
        centerPanel = new CenterPanel();
        roomListPanel = new RoomsListCenterPanel();

        mainPanel.add(roomListPanel, BorderLayout.NORTH);
        mainPanel.add(leftPanel, BorderLayout.WEST);
        mainPanel.add(rightPanel, BorderLayout.EAST);
        mainPanel.add(centerPanel, BorderLayout.CENTER);

        roomListPanel.highlightRoom(selectedRoomIndex);
        remove(gamePanel);
        add(mainPanel, BorderLayout.SOUTH);
        revalidate();
        repaint();
        
        multiplier.resetMultiplier();
    }

    public void getGameRoomPanel() {
        mainPanel.remove(leftPanel);
        mainPanel.remove(rightPanel);
        mainPanel.remove(centerPanel);
        mainPanel.remove(roomListPanel);
        gRoomLeftPanel = new GameRoomLeftPanel();
        gRoomCentralPanel = new GameRoomCenterPanel();
        gRoomRightPanel = new GameRoomRightPanel();
        mainPanel.add(gRoomLeftPanel, BorderLayout.WEST);
        mainPanel.add(gRoomCentralPanel, BorderLayout.CENTER);
        mainPanel.add(gRoomRightPanel, BorderLayout.EAST);
        add(gamePanel, BorderLayout.CENTER);
        // add(mainPanel, BorderLayout.SOUTH);
        revalidate();
        repaint();
    }


    public void multiplierStopped() {
        System.out.println("Multiplier stopped called");
        gameInProgress = false;
        if (gameStarted){
            multiplier.setIsStopped(true);
            // Add the explosion, remove the player
            multiplier.resetMultiplier();
            System.out.println("Explosion here");
            gamePanel.remove(player);
            if(userJoinedRound && !cashoutPressed){
                mainPanel.remove(cashoutCenterPanel);
                add(mainPanel, BorderLayout.SOUTH);
                revalidate();
                repaint();
                //Remover cashout
            }
            cashoutPressed = false;
                // explosion = new Explosion();
                // gamePanel.add(explosion);
                // mainPanel.add(gamePanel, BorderLayout.NORTH);
            gamePanel.revalidate();
            gamePanel.repaint();
            // Remove the explosion & multiplier, add the countdown panel
            // Timer timer = new Timer(10000, e -> {
            //     System.out.println("Remove Explosion");
            //     // mainPanel.remove(explosion);
            gamePanel.remove(multiplier);
            gamePanel.add(countdownPanel, BorderLayout.NORTH);
            countdownPanel.resetTime();
            gamePanel.add(player, BorderLayout.CENTER);
            gamePanel.revalidate();
            gamePanel.repaint();
            // });
            // timer.setRepeats(false);
            // timer.start();
        }
        mainPanel.add(gRoomLeftPanel, BorderLayout.WEST);
        mainPanel.add(gRoomCentralPanel, BorderLayout.CENTER);
        mainPanel.add(gRoomRightPanel, BorderLayout.EAST);

        mainPanel.revalidate();
        mainPanel.repaint();
    }
    public void resetCountdown(){
        if (gameStarted){
            System.out.println("Reset countdown called");
        }
        
    }
    public void startGame() {

        multiplier.resetMultiplier();
        if (gameStarted == true) {
            gamePanel.remove(countdownPanel);
            gamePanel.revalidate();
            gamePanel.repaint();
        }
        gameStarted = true;
        gameInProgress = true;
        removeBetButtonsNoCashout();
        multiplier.setIsStopped(false);
        gamePanel.add(player, BorderLayout.CENTER);
        gamePanel.add(multiplier, BorderLayout.NORTH);
        gamePanel.revalidate();
        gamePanel.repaint();
        System.out.println("Added game panel");

    }

    JLabel selectedRoomLabel = new JLabel("Select a room");

    public void updateSelectedRoomLabel() {
        if (roomListStrings.length > 0 && selectedRoomIndex >= 0 && selectedRoomIndex < roomListStrings.length) {
            // Now it's safe to access roomListStrings[selectedRoomIndex]
            selectedRoomLabel.setText(roomListStrings[selectedRoomIndex]);
        } else {
            // Either no rooms are available or selectedRoomIndex is out of bounds
            selectedRoomLabel.setText("No rooms available");
        }
    }


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
                        Socket socket = new Socket(SERVER_ADDRESS, SERVER_PORT);
                        OutputStream outputStream = socket.getOutputStream();

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
                        // connect to the server
                        Socket socket = new Socket(SERVER_ADDRESS, SERVER_PORT);
                        InputStream inputStream = socket.getInputStream();
                        OutputStream outputStream = socket.getOutputStream();

                        String initializeMessage = "GET_ACTIVE_ROOMS";
                        outputStream.write(initializeMessage.getBytes());

                        ArrayList<String> roomList = new ArrayList<>();

                        // Read the message from the server
                        byte[] buffer = new byte[1024];
                        int bytesRead = inputStream.read(buffer);
                        if (bytesRead != -1) {
                            String response = new String(buffer, 0, bytesRead);
                            String[] roomNames = response.split(" "); // Split the response string by spaces

                            // Add each room name to the list
                            for (String roomName : roomNames) {
                                roomList.add(roomName);
                            }
                            
                            if (roomList.isEmpty()) {
                                roomList.add("No rooms available");
                            }
    
                            // Now, update roomListStrings and refresh UI accordingly
                            System.out.println("roomList after creating new room: " + roomList);
                            roomListStrings = roomList.toArray(new String[0]);
                        } else {
                            System.out.println("No data received from server");
                        }
                        socket.close();
                        
                        updateRoomListUI();
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
                    updateStatus();
                    System.out.println("Selected room: " + roomNumber);
                    try {
                        Socket socket = new Socket(SERVER_ADDRESS, SERVER_PORT);
                        OutputStream outputStream = socket.getOutputStream();

                        roomNumber = roomListStrings[selectedRoomIndex];

                        String request = "JOIN_ROOM " + roomNumber + " " + userId;

                        System.out.println("Join room request: " + request);
                        outputStream.write(request.getBytes());

                        ServerListener listener = new ServerListener(socket);
                        listenerThread = new Thread(listener);
                        listenerThread.start();
                    } catch (IOException ex) {
                        ex.printStackTrace();
                    }
                    getGameRoomPanel();
                }
            });
            add(selectRoomButton, selectButtonConstraints);
        }
    }
    
    // bet - up and down arrow with bet balance
    // cash out
    // leave room

    // Step 1: ServerListener class
    class ServerListener implements Runnable {
        private Socket socket;
        private volatile boolean running = true; // To control the running of the thread

        public ServerListener(Socket socket) {
            this.socket = socket;
        }

        public void run() {
            try {
                InputStream inputStream = socket.getInputStream();
                byte[] buffer = new byte[1024];
                int bytesRead;

                while (running && (bytesRead = inputStream.read(buffer)) != -1) {
                    String message = new String(buffer, 0, bytesRead);
                    handleServerMessage(message);
                }
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                try {
                    socket.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

        private void handleServerMessage(String message) {
            if (message.startsWith("START_GAME")) {
                SwingUtilities.invokeLater(() -> {
                    System.out.println("START_GAME CALLED");
                    // if (gameInitialized) {
                    //     multiplierStopped();
                    // } else {
                    //     startGame();
                    //     gameInitialized=true;
                    // }
                    
                    startGame();
                });
                System.out.println("Game started.");
            } else if (message.startsWith("END_GAME")) {
                // Handle END_GAME
                System.out.println("Game ended.");
                multiplierStopped();
            } else if (message.startsWith("BALANCE")) {
                // Extract and update balance
                String[] parts = message.split(" ");
                if (parts.length > 1) {
                    String balance = parts[1];
                    cashAmount = Double.parseDouble(balance);
                    betAmount = 1;
                    updateCashAmount(cashAmount);
                    updateBetAmount(betAmount);
                    System.out.println("Balance updated: " + balance);
                }
            }
        }

        // Call this method to stop the thread
        public void stopListening() {
            running = false;
        }
    }
      


    public void updateRoomListUI() {
        // Remove existing RoomsListCenterPanel if present
        if (roomListPanel != null) {
            mainPanel.remove(roomListPanel);
        }
    
        // Create a new RoomsListCenterPanel with updated roomListStrings
        roomListPanel = new RoomsListCenterPanel();
        mainPanel.add(roomListPanel, BorderLayout.NORTH);
    
        // Ensure that we don't try to highlight a room if "No rooms" is the only entry
        if (roomListStrings.length > 0 && !roomListStrings[0].equals("No rooms available")) {
            selectedRoomIndex = Math.max(0, Math.min(selectedRoomIndex, roomListStrings.length - 1)); // Ensure index is within bounds
            roomListPanel.highlightRoom(selectedRoomIndex);
        } else {
            selectedRoomIndex = -1; // Indicates no selection is possible
        }
    
        revalidate();
        repaint();
    }
    

    public static void initializeUser() {
        // Logic for creating user --> join the server through a socket connection
        // Should receive a "userId" (string) from the server. Store this in a global variable
        try {
            // connect to the server
            Socket socket = new Socket(SERVER_ADDRESS, SERVER_PORT);

            // input stream to get response
            InputStream inputStream = socket.getInputStream();

            // output stream to send message
            OutputStream outputStream = socket.getOutputStream();

            // Send the "INITIALIZE_USER" message to the server
            String initializeMessage = "INITIALIZE_USER";
            outputStream.write(initializeMessage.getBytes());

             // Read the message from the server
            byte[] buffer = new byte[1024];
            int bytesRead = inputStream.read(buffer);
            if (bytesRead != -1) {
                String user_id = new String(buffer, 0, bytesRead);
                userId = user_id; // set the global user id
                System.out.println("Received user ID from server: " + userId);
            } else {
                System.out.println("No data received from server");
            }
            socket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        // User handling to add user -> create user
        initializeUser();
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
