#include <vector>
#include <iostream>
#include <fstream>


#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Rect.H>
#include <FL/fl_draw.H>
#include <FL/fl_Color_Chooser.H>
#include <Fl/Fl_Menu_Bar.H>
#include <Fl/Fl_File_Chooser.H>



struct Color {
    int r;
    int g;
    int b;
};

class Image {
    private:
        int width;
        int height;
        std::vector<Color> pixels;
    public:
        Image (int w, int h);
        void setPixel (int x, int y, const Color& color);
        Color getPixel (int x, int y) const;
        void savePPM (const std::string& filename) const;
        void loadPPM (const std::string& filename);
        
        int getWidth() const;
        int getHeight() const;
};

Image::Image (int w, int h) : width(w), height(h) {
    pixels.resize(width * height, {0, 0, 0});
}

void Image::setPixel (int x, int y, const Color& color) {
    pixels[y * width + x] = color;
}

void Image::savePPM (const std::string& filename) const {
    std::ofstream out (filename);
    
    out << "P3\n";
    out << width << " " << height << "\n";
    out << "255\n";
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Color c = getPixel(x, y);
            out << c.r << " " << c.g << " " << c.b << " ";
        }
        out << '\n';
    }
}

void Image::loadPPM (const std::string& filename) {
    std::ifstream in (filename);
    if (!in) {
        throw std::runtime_error ("Cannot open file: " + filename);
    }
    
    std::string magic;
    in >> magic;
    
    if (magic != "P3") {
        std::cout << "||" << magic << "||" << std::endl;
        throw std::runtime_error ("Not a P3 PPM file!");
    }
    
    in >> width >> height;
    int maxColor;
    in >> maxColor;
    
    pixels.resize(width * height);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Color c;
            in >> c.r >> c.g >> c.b;
            setPixel(x, y, c);
        }
    }
}






Color Image::getPixel (int x, int y) const {
    return pixels[y * width + x];
}

int Image::getWidth() const {
    return width;
}

int Image::getHeight() const {
    return height;
}


class ImageWidget : public Fl_Widget {
    private:
        Image *image;
        int cellSize;
        Color currentColor;
    public:
        ImageWidget (int x, int y, int w, int h, Image *img) :
            Fl_Widget (x, y, w, h), image (img) {
                currentColor = {255, 0, 0};
            }
        
        void setCurrentColor (const Color& c) {
            currentColor = c;
        }
        
        void draw () override {
            
            
            int widgetWidth = w();
            int widgetHeight = h();

            int newCellSizeX = widgetWidth / image->getWidth();
            int newCellSizeY = widgetHeight / image->getHeight();
            cellSize = std::min(newCellSizeX, newCellSizeY);
            
            for (int y = 0; y < image->getHeight(); y++) {
                for (int x = 0; x < image->getWidth(); x++) {
                    Color c = image->getPixel(x, y);
                    
                    fl_color (c.r, c.g, c.b);
                    fl_rectf (
                        this->x() + x * cellSize,
                        this->y() + y * cellSize,
                        cellSize,
                        cellSize
                    );
//                    fl_color (255, 255, 255);
//                    fl_rect(
//                        this->x() + x * cellSize,
//                        this->y() + y * cellSize,
//                        cellSize,
//                        cellSize
//                    );
                }
            }
        }
        int handle (int event) override {
            switch (event) {
                case FL_PUSH:
                case FL_DRAG:
                    int mouseX = Fl::event_x();
                    int mouseY = Fl::event_y();
                    
                    int gridX = (mouseX - x()) / cellSize;
                    int gridY = (mouseY - y()) / cellSize;
                    
                    if (gridX >= 0 && gridX < image->getWidth() && 
                        gridY >= 0 && gridY < image->getHeight()) {
                            image->setPixel(gridX, gridY, currentColor);
                            redraw();
                        }
                    return 1;
            }
            return Fl_Widget::handle(event);
        }
        Image* getImage() {
            return image;
        }
};


void chooseColor (Fl_Widget *w, void *data) {
    ImageWidget *widget = (ImageWidget *)data;
    
    double r = 1.0;
    double g = 0.0;
    double b = 0.0;
    
    if (fl_color_chooser ("Pick Color", r, g, b)) {
        Color c;
        c.r = (int) (r * 255);
        c.g = (int) (g * 255);
        c.b = (int) (b * 255);
        widget->setCurrentColor(c);
    }
    
}

void load (Fl_Widget *w, void *data) {
    ImageWidget *widget = (ImageWidget *)data;
    Image *img = widget->getImage();
    
    const char *filename = fl_file_chooser ("Open PPM File", "*.ppm", "");
    if (filename) {
        img->loadPPM(filename);
        widget->redraw();
    }
    
}

void save (Fl_Widget *w, void *data) {
    ImageWidget *widget = (ImageWidget *)data;
    Image *img = widget->getImage();
    
    const char *filename = fl_file_chooser("Save PPM File", "*.ppm", "test.ppm");
    
    if (filename) {
        img->savePPM(filename);
    }
    
}

int main (int argc, char ** argv) {
    Fl_Window *window = new Fl_Window(1200, 800, "AnotherPixEditor");
    Fl::scheme("oxy");
    
    Image img(32, 32);
    ImageWidget widget(0, 30, 600, 600, &img);
    
    Fl_Menu_Bar *menubar = new Fl_Menu_Bar(0, 0, 1200, 30);
    menubar->add("File/Open", 0, load, &widget);
    menubar->add("File/Save", 0, save, &widget);
    
    
    Fl_Group *content = new Fl_Group(0, 30, 1200, 770);

    
    img.setPixel(1, 1, {255, 0, 0});
    img.setPixel(2, 2, {0, 255, 0});
    img.setPixel(3, 3, {0, 0, 255});
    
    Fl_Button colorBtn (700, 210, 100, 30, "Pick Color");
    colorBtn.callback (chooseColor, &widget);
    
    
    
    
    content->end();
    
    window->resizable(content);
    
    
    window->end();
    window->show (argc, argv);
    
    return Fl::run();
}
