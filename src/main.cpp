#include <vector>
#include <iostream>
#include <fstream>


#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Rect.H>
#include <FL/fl_draw.H>
#include <FL/fl_Color_Chooser.H>
#include <Fl/Fl_Menu_Bar.H>
#include <Fl/Fl_File_Chooser.H>
#include <Fl/Fl_Scroll.H>


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
        bool showGrid;
        
        
        
    public:
        ImageWidget (int x, int y, int w, int h, Image *img) :
            Fl_Widget (x, y, w, h), image (img), cellSize(20) {
                currentColor = {255, 0, 0};
                showGrid = false;
                updateSize();
                box(FL_BORDER_BOX);       // experimental
                color(FL_BLACK);        // experimental
            }
        
        void setCurrentColor (const Color& c) {
            currentColor = c;
        }
        
        void setShowGrid (bool value) {
            showGrid = value;
            redraw();
        }
        
        
        
        
        
        // experimental
        void updateSize () {
            resize(x(), y(), image->getWidth() * cellSize, image->getHeight() * cellSize);
        }
        void setCellSize(int size) {
            if (size < 2) {
                size = 2;
            }
            if (size > 100) {
                size = 100;
            }
            
            cellSize = size;
            updateSize();
            redraw();
            
            if(parent()) parent()->redraw();
            
        }
        int getCellSize() const {
            return cellSize;
        }
        
        
        
        void draw () override {
            
            draw_box();
//            int widgetWidth = w();
//            int widgetHeight = h();
//
//            int newCellSizeX = widgetWidth / image->getWidth();
//            int newCellSizeY = widgetHeight / image->getHeight();
//            cellSize = std::min(newCellSizeX, newCellSizeY);
            
            
            
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
                    
                    if (showGrid) {
                        fl_color(200, 200, 200);
                        fl_rect(
                            this->x() + x * cellSize,
                            this->y() + y * cellSize,
                            cellSize,
                            cellSize
                        );
                    }
                }
            }
        }
        int handle (int event) override {
            switch (event) {
                case FL_PUSH:
                case FL_DRAG: {
                    int mouseX = Fl::event_x();
                    int mouseY = Fl::event_y();
                    
                    int gridX = (mouseX - x()) / cellSize;
                    int gridY = (mouseY - y()) / cellSize;
                    
                    if (gridX >= 0 && gridX < image->getWidth() && 
                        gridY >= 0 && gridY < image->getHeight()) {
                            image->setPixel(gridX, gridY, currentColor);
                            
                            int px = x() + gridX * cellSize;
                            int py = y() + gridY * cellSize;
                            
                            damage(FL_DAMAGE_USER1, px, py, cellSize, cellSize);
                            
                            
//                            redraw();
                        }
                    return 1;
                }

                case FL_MOUSEWHEEL: {
                    int dy = Fl::event_dy();
                    if (dy < 0) {
                        setCellSize(cellSize + 2);
                    }
                    else {
                        setCellSize(cellSize - 2);
                    }
                    return 1;
                }

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
        widget->updateSize();
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

void gridToggle(Fl_Widget *w, void *data) {
    ImageWidget* widget = (ImageWidget*)data;
    Fl_Light_Button* btn = (Fl_Light_Button*)w;

    widget->setShowGrid(btn->value());
}

void Oneraser(Fl_Widget *w, void *data) {
    ImageWidget *widget = (ImageWidget *)data;
    widget->setCurrentColor({0, 0, 0});
}


int main (int argc, char ** argv) {
    Fl_Window *window = new Fl_Window(1200, 800, "AnotherPixEditor");
    Fl::scheme("oxy");
    
    // menu bar
     Fl_Menu_Bar *menubar = new Fl_Menu_Bar(0, 0, 1200, 30);
    
    
    Image img(8, 8);
    ImageWidget widget(20, 50, img.getWidth() * 20, img.getHeight() * 20, &img);
    

    Fl_Scroll *scroll = new Fl_Scroll(0, 30, 800, 700);
    scroll->box(FL_FLAT_BOX);
    scroll->end();
    scroll->add(widget);
    
    
    Fl_Group *rightPanel = new Fl_Group(810, 30, 380, 770);
    rightPanel->box(FL_BORDER_BOX);
    rightPanel->resizable(0);
    
    Fl_Light_Button *grid_ON_OFF = new Fl_Light_Button(830, 60, 100, 30, "Gridline");
    grid_ON_OFF->callback(gridToggle, &widget);
    Fl_Button *eraser = new Fl_Button(830, 100, 100, 30, "Eraser");
    eraser->callback(Oneraser, &widget);
    
    rightPanel->end();
    
   
    menubar->add("File/Open" ,FL_CTRL + 'o', load, &widget);
    menubar->add("File/Save", FL_CTRL + 's', save, &widget);
    menubar->add("Color", 0, chooseColor, &widget);

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    window->resizable(scroll);
    window->end();
    window->show (argc, argv);
    
    return Fl::run();
}
