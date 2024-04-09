package components;
import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class Multiplier extends JPanel {
    private JLabel multiplierLabel;
    private Timer timer;
    private double multiplier = 1.0;
    private int initialDelay = 1000;
    private double speedIncrease = 0.95;
    private MultiplierListener multiplierListener;
    private boolean isStopped = false;

    public Multiplier() {
        setLayout(new BorderLayout());

        multiplierLabel = new JLabel("Multiplier: 1.00x");
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

    public void resetMultiplier(){
        multiplier = 1.0;
        speedIncrease = 0.95;
        multiplierLabel.setText("Multiplier: " + String.format("%.2fx", multiplier));
        isStopped = true;
        timer = null;
        timer = new Timer(initialDelay, new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                updateMultiplier();
            }
        });
        timer.start();
    }

    public void setIsStopped(boolean isStopped){
        this.isStopped = isStopped;
    }

    // TODO: @Tjin
    private void updateMultiplier() {
        if (!isStopped) {
            multiplier += 0.1;
            int newDelay = (int) (timer.getDelay() * speedIncrease);
            timer.setDelay(newDelay);
            multiplierLabel.setText("Multiplier: " + String.format("%.2fx", multiplier));
        }
    }
    public void addMultiplierListener(MultiplierListener listener) {
        this.multiplierListener = listener;
    }

    public void fireMultiplierStoppedEvent() {
        if (multiplierListener != null) {
            multiplierListener.multiplierStopped();
        }
    }

    // Custom listener interface
    public interface MultiplierListener {
        void multiplierStopped();
    }
}
