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
            Fl_Widget (x, y, w, h), image (img), cellSize(20) 
            {
                currentColor = {255, 255, 255}; // white
                showGrid = false;
                updateSize();
                box(FL_BORDER_BOX);       // experimental
            }
        
        //----------------------------------------
        void draw () override;
        int handle (int event) override;
        void updateSize ();
        
        void setCurrentColor (const Color& c);
        void setShowGrid (bool value);
        void setCellSize (int size);
        
        int getCellSize() const;
        Image* getImage();
        
        //----------------------------------------
        
        
        

};









void ImageWidget::setCurrentColor (const Color& c) {
    currentColor = c;
}

void ImageWidget::setShowGrid (bool value) {
    showGrid = value;
    redraw();
}

// experimental
void ImageWidget::updateSize () {
    resize(x(), y(), image->getWidth() * cellSize, image->getHeight() * cellSize);
    redraw();
}







void ImageWidget::setCellSize(int size) {
    if (size < 2) {
        size = 2;
    }
    if (size > 100) {
        size = 100;
    }
    
    cellSize = size;
    updateSize();
    
    if(parent()) parent()->redraw();
    
}

int ImageWidget::getCellSize() const {
    return cellSize;
}



void ImageWidget::draw () {
    
//    draw_box();
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
int ImageWidget::handle (int event) {
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
Image* ImageWidget::getImage() {
    return image;
}





struct uiData {
    ImageWidget *widget;
    Fl_Box *preview;
};














void chooseColor (Fl_Widget *w, void *data) {
    uiData *ui = (uiData*)data;
    
    double r = 1.0;
    double g = 0.0;
    double b = 0.0;
    
    if (fl_color_chooser ("Pick Color", r, g, b)) {
        Color c;
        c.r = (int) (r * 255);
        c.g = (int) (g * 255);
        c.b = (int) (b * 255);
        
        ui->widget->setCurrentColor(c);
        
        ui->preview->color (fl_rgb_color(c.r, c.g, c.b));
        ui->preview->redraw();
    }
    
}

void load (Fl_Widget *w, void *data) {
    ImageWidget *widget = (ImageWidget *)data;
    Image *img = widget->getImage();
    
    const char *filename = fl_file_chooser ("Open PPM File", "*.ppm", "");
    if (filename) {
        img->loadPPM(filename);
        widget->updateSize();
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
    uiData *ui = (uiData*)data;
    ui->widget->setCurrentColor({0, 0, 0});
    ui->preview->color(fl_rgb_color(0,0,0));
    ui->preview->redraw();
}









int main (int argc, char ** argv) {
    Fl::scheme("oxy");
    Fl_Window *window = new Fl_Window(1200, 800, "AnotherPixEditor");
    
    // menu bar
    Fl_Menu_Bar *menubar = new Fl_Menu_Bar(0, 0, 1200, 30);
    
    
    Image *img = new Image(16, 16);
    
    
    
    
    
    
    Fl_Group *leftPanel = new Fl_Group(0, 30, 250, 770);
    leftPanel->box(FL_BORDER_BOX);
    leftPanel->resizable(0);
    //----------------------------------------------------------------------------------------------
    Fl_Light_Button *grid_ON_OFF = new Fl_Light_Button(10, 40, 100, 30, "Gridline");
    Fl_Button *eraser = new Fl_Button (10, 70, 100, 30, "Eraser");
    
    Fl_Box *preview = new Fl_Box(70, 110, 120, 25);
    preview->box(FL_BORDER_BOX);
    preview->color(fl_rgb_color(255,255,255));
    Fl_Box *previewColorLabel = new Fl_Box (10, 110, 60, 25, "Color:");
    previewColorLabel->labelfont(FL_BOLD);
    
    
    //----------------------------------------------------------------------------------------------
    leftPanel->end();
    
   
   
   
   
    Fl_Scroll *scroll = new Fl_Scroll(280, 50, 2 * img->getWidth() * 20, 2 * img->getHeight() * 20);
    
    ImageWidget *widget = new ImageWidget(280, 50,
                                       img->getWidth() * 20,
                                       img->getHeight() * 20,
                                       img);
    
    scroll->end();
    
    uiData *ui = new uiData {widget, preview};
    
    
    
    
    grid_ON_OFF->callback(gridToggle, widget);
    eraser->callback(Oneraser, ui);
    menubar->add("File/Open" ,FL_CTRL + 'o', load, widget);
    menubar->add("File/Save", FL_CTRL + 's', save, widget);
    menubar->add("Color", 0, chooseColor, ui);
    
    
    
    
    window->resizable(scroll);
    window->end();
    window->show (argc, argv);
    
    return Fl::run();
}
