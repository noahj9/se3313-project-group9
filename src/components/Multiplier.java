package components;
import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class Multiplier extends JPanel {
    private JLabel multiplierLabel;
    private Timer timer;
    private double multiplier;
    private int initialDelay = 1000;
    private double speedIncrease = 0.95;
    private MultiplierListener multiplierListener;

    public Multiplier() {
        setLayout(new BorderLayout());

        multiplierLabel = new JLabel("Multiplier: 0.00x");
        multiplierLabel.setHorizontalAlignment(JLabel.CENTER);
        multiplierLabel.setFont(new Font("Arial", Font.BOLD, 24));
        add(multiplierLabel, BorderLayout.CENTER);

        timer = new Timer(initialDelay, new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                updateMultiplier();
            }
        });
        timer.start();
    }

    public double getMultiplier(){
        return multiplier;
    }

    private void updateMultiplier() {
        multiplier+=0.01;
        if (Math.random() < 0.005) {
            timer.stop();
            fireMultiplierStoppedEvent();
        } else {
            int newDelay = (int) (timer.getDelay() * speedIncrease);
            timer.setDelay(newDelay);
        }
        multiplierLabel.setText("Multiplier: " + String.format("%.2fx", multiplier));
    }
    public void addMultiplierListener(MultiplierListener listener) {
        this.multiplierListener = listener;
    }

    private void fireMultiplierStoppedEvent() {
        if (multiplierListener != null) {
            multiplierListener.multiplierStopped();
        }
    }

    // Custom listener interface
    public interface MultiplierListener {
        void multiplierStopped();
    }
}
