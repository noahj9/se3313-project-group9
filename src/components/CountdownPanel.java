package components;
import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class CountdownPanel extends JPanel {
    private JLabel countdownLabel;
    private int time;
    private CountdownListener countdownListener;
    public CountdownPanel() {
        setLayout(new BorderLayout());

        countdownLabel = new JLabel("Enter the Round. Closes in: 10s");
        countdownLabel.setHorizontalAlignment(JLabel.CENTER);
        countdownLabel.setFont(new Font("Arial", Font.BOLD, 24));
        add(countdownLabel, BorderLayout.CENTER);
    }
    public void resetTime(){
        time = 8;
        Timer timer = new Timer(1000, new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                countdownLabel.setText("Enter the Round. Closes in: " + time +"s");
                time--;

                if (time < 0) {
                    ((Timer) e.getSource()).stop();
                    countdownLabel.setText("Round Closed");
                    // fireCountdownFinishedEvent();
                }
            }
        });
        timer.start();
    }
    public int getTime() {
        return time;
    }

    public void addCountdownListener(CountdownListener listener) {
        this.countdownListener = listener;
    }

    private void fireCountdownFinishedEvent() {
        if (countdownListener != null) {
            countdownListener.resetCountdown();
        }
    }

    public interface CountdownListener {
        void resetCountdown();
    }
}
