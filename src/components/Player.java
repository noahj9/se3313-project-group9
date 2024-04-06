package components;

import javax.swing.*;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.IOException;
import javax.imageio.ImageIO;

public class Player extends JPanel {
    private BufferedImage[] images;
    private int index;
    private Timer timer;

    public Player() {
        images = new BufferedImage[4];
        try {
            images[0] = ImageIO.read(getClass().getResource("images/spaceman/tile008.png"));
            images[1] = ImageIO.read(getClass().getResource("images/spaceman/tile009.png"));
            images[2] = ImageIO.read(getClass().getResource("images/spaceman/tile010.png"));
            images[3] = ImageIO.read(getClass().getResource("images/spaceman/tile011.png"));
        } catch (IOException e) {
            e.printStackTrace();
        }

        index = 0;
        timer = new Timer(100, e -> {
            index = (index + 1) % images.length;
            repaint();
        });
        timer.start();
    }

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        if (images[index] != null) {
            int newWidth = images[index].getWidth() * 2;
            int newHeight = images[index].getHeight() * 2;
            g.drawImage(images[index], 350, 80, newWidth, newHeight, this); // MAKE SURE THIS MATCHES EXPLOSION X/Y VALUES
        }
    }
}
