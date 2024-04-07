import javax.swing.*;
import java.awt.*;

import components.ControllerGame;

public class MainScreen extends JFrame {

    public MainScreen() {
        super("Combined Frame");

        Panel panel = new Panel();
        add(panel, BorderLayout.NORTH);

        // Create and add the ControllerGame at the bottom
        ControllerGame controllerGame = new ControllerGame();
        add(controllerGame, BorderLayout.SOUTH);

        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setSize(500, 400); // Set appropriate size
        setVisible(true);
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(MainScreen::new);
    }
}
