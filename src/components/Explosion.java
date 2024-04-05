package components;
import javax.swing.*;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.IOException;
import javax.imageio.ImageIO;

public class Explosion extends JPanel{
    private BufferedImage[] images;
    private int index;
    private Timer timer;

    public Explosion() {
        images = new BufferedImage[16];
        try {
            images[0] = ImageIO.read(getClass().getResource("images/explosion/tile000.png"));
            images[1] = ImageIO.read(getClass().getResource("images/explosion/tile001.png"));
            images[2] = ImageIO.read(getClass().getResource("images/explosion/tile002.png"));
            images[3] = ImageIO.read(getClass().getResource("images/explosion/tile003.png"));
            images[4] = ImageIO.read(getClass().getResource("images/explosion/tile004.png"));
            images[5] = ImageIO.read(getClass().getResource("images/explosion/tile005.png"));
            images[6] = ImageIO.read(getClass().getResource("images/explosion/tile006.png"));
            images[7] = ImageIO.read(getClass().getResource("images/explosion/tile007.png"));
            images[8] = ImageIO.read(getClass().getResource("images/explosion/tile008.png"));
            images[9] = ImageIO.read(getClass().getResource("images/explosion/tile009.png"));
            images[10] = ImageIO.read(getClass().getResource("images/explosion/tile010.png"));
            images[11] = ImageIO.read(getClass().getResource("images/explosion/tile011.png"));
            images[12] = ImageIO.read(getClass().getResource("images/explosion/tile012.png"));
            images[13] = ImageIO.read(getClass().getResource("images/explosion/tile013.png"));
            images[14] = ImageIO.read(getClass().getResource("images/explosion/tile014.png"));
            images[15] = ImageIO.read(getClass().getResource("images/explosion/tile015.png"));


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
            g.drawImage(images[index], 350, 80, newWidth, newHeight, this); // MAKE SURE THIS MATCHES PERSON X/Y
        }
    }

}
