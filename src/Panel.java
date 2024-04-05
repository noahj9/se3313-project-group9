import javax.swing.*;
import java.awt.*;
import components.CountdownPanel;
import components.Player;
import components.Explosion;
import components.Multiplier;

public class Panel extends JPanel implements CountdownPanel.CountdownListener, Multiplier.MultiplierListener {
    private CountdownPanel countdownPanel;
    private Multiplier multiplier;
    private Player player;
    private Explosion explosion;

    public Panel() {
        setLayout(new BorderLayout());
        countdownPanel = new CountdownPanel();
        countdownPanel.addCountdownListener(this);
        add(countdownPanel, BorderLayout.NORTH);
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

    public static void main(String[] args) {
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
