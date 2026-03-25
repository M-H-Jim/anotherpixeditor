#include <vector>
#include <cmath>
#include <queue>
#include <iostream>
#include <fstream>
#include <algorithm>


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
#include <FL/fl_show_colormap.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Pack.H>
#include <FL/fl_Flex.H>
#include <FL/Fl_Tabs.H>


const int MAX_UNDO = 50;



class HoverButton : public Fl_Button {
    public:
        HoverButton (int x, int y, int w, int h, const char *L = 0) : Fl_Button (x, y, w, h, L) {}
        int handle (int event) override {
            switch (event) {
                case FL_ENTER:
                    color(fl_rgb_color(120, 135, 155));
                    redraw();
                    return 1;
                case FL_LEAVE:
                    color(fl_rgb_color(92, 103, 117));
                    redraw();
                    return 1;
            }
            return Fl_Button::handle(event);
        } 
};
class HoverLightButton : public Fl_Light_Button {
    public:
        HoverLightButton (int x, int y, int w, int h, const char *L = 0) : Fl_Light_Button (x, y, w, h, L) {}
        int handle (int event) override {
            switch (event) {
                case FL_ENTER:
                    color(fl_rgb_color(120, 135, 155));
                    redraw();
                    return 1;
                case FL_LEAVE:
                    color(fl_rgb_color(92, 103, 117));
                    redraw();
                    return 1;
            }
            return Fl_Light_Button::handle(event);
        } 
};
class HoverMenu : public Fl_Menu_Bar {
    public:
        HoverMenu (int x, int y, int w, int h, const char *L = 0) : Fl_Menu_Bar (x, y, w, h, L) {}
        int handle (int event) override {
            switch (event) {
                case FL_ENTER:
                    color(fl_rgb_color(58, 66, 82));
                    selection_color(fl_rgb_color(94, 156, 255));
                    redraw();
                    return 1;
                case FL_LEAVE:
                    color(fl_rgb_color(36, 40, 47));
//                    selection_color(fl_rgb_color(77, 138, 255));
                    redraw();
                    return 1;
            }
            return Fl_Menu_Bar::handle(event);
        } 
};








struct Color {
    int r;
    int g;
    int b;
    
    bool operator ==(const Color& other) const {
        return r == other.r && g == other.g && b == other.b;
    }
    
};  // This structure represents the pixels colors

Color convertColor (Fl_Color c) {
    unsigned char r, g, b;
    Fl::get_color(c, r, g, b);
    return {r, g, b};
}


class Image {
    private:
        int width;
        int height;
        std::vector<Color> pixels;
    public:
        Image (int w, int h);
        Image (const Image& rval);
        void setPixel (int x, int y, const Color& color);
        Color getPixel (int x, int y) const;
        void savePPM (const std::string& filename) const;
        void loadPPM (const std::string& filename);
        
        int getWidth() const;
        int getHeight() const;
};

Image::Image (int w, int h) : width(w), height(h) {
    pixels.resize(width * height, {255, 255, 255});
}

Image::Image (const Image& rval) {
    width = rval.width;
    height = rval.height;
    pixels = rval.pixels;
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
    
    public:
        enum Tool {
            TOOL_PEN,
            TOOL_LINE,
            TOOL_CIRCLE,
            TOOL_BUCKET,
            TOOL_RECTANGLE
        };
    
    
    private:
        Image *image;
        int cellSize;
        Color currentColor;
        bool showGrid;
        bool mirrorHorizontal;
        bool mirrorVertical;
        
        Fl_Box *previewBox = nullptr;
        
        std::vector<Image> undoStack;
        std::vector<Image> redoStack;
        
        int brushSize;
        
        bool middleDrag;
        int dragLastX;
        int dragLastY;
        
        Fl_Color_Chooser *colorChooser = nullptr;
        
        Tool currentTool;
        int startX;
        int startY;
        int endX;
        int endY;
        int previewX;
        int previewY;
        bool isDrawingShape;
        
        
        
        void updateStatus();
        
        
        
        
    public:
        ImageWidget (int x, int y, int w, int h, Image *img) :
            Fl_Widget (x, y, w, h), image (img), cellSize(20) 
            {
                
                
                currentColor = {0, 0, 0}; // Black
                showGrid = false;
                mirrorHorizontal = false;
                mirrorVertical = false;
                updateSize();
                box(FL_BORDER_BOX);       // experimental
                currentTool = TOOL_PEN;
                startX = startY = 0;
                endX = endY = 0;
                previewX = previewY = 0;
                isDrawingShape = false;
                brushSize = 1;
                middleDrag = false;
                dragLastX = dragLastY = 0;
                
                
                
            }
        
        //----------------------------------------
        void draw () override;
        int handle (int event) override;
        void drawAtMouse (int mouseX, int mouseY);
        void drawPixelCell (int gx, int gy);
        void pickColorAtMouse(int mouseX, int mouseY);
        void updateSize ();
        void undo();
        void redo();
        
        
        void setLinePixels (int x0, int y0, int x1, int y1);
        void drawCirclePoints (int cx, int cy, int x, int y);
        void set8CirclePixels (int cx, int cy, int x, int y);
        void safeSetPixel (int x, int y);
        void setCirclePixels (int cx, int cy, int r);
        
        void floodFill (int x, int y);
        
        
        void flipHorizontal();
        void flipVertical();
        
        void setBrushSize (int s);
        void setCurrentColor (const Color& c);
        void setShowGrid (bool value);
        void setMirrorHorizontal (bool value);
        void setMirrorVertical (bool value);
        void setCellSize (int size);
        void setCurrentTool (Tool t);
        void setPreviewBox (Fl_Box *box);
        void setColorChooser(Fl_Color_Chooser *chooser);
        
        int getCellSize() const;
        Color getCurrentColor() const;
        Image* getImage();
        
        //----------------------------------------

};








void ImageWidget::setCurrentColor (const Color& c) {
    currentColor = c;
    if (previewBox) {
        previewBox->color(fl_rgb_color(c.r, c.g, c.b));
        previewBox->redraw();
    }
    if (colorChooser) {
        colorChooser->color(fl_rgb_color(c.r, c.g, c.b));
        colorChooser->rgb(c.r / (double)255, c.g / (double)255, c.b / (double)255);
        colorChooser->redraw();
    }
}

void ImageWidget::setShowGrid (bool value) {
    showGrid = value;
    redraw();
}

void ImageWidget::setMirrorHorizontal (bool value) {
    mirrorHorizontal = value;
}

void ImageWidget::setMirrorVertical (bool value) {
    mirrorVertical = value;
}

void ImageWidget::setPreviewBox(Fl_Box *box) {
    previewBox = box;
}

void ImageWidget::setColorChooser(Fl_Color_Chooser *chooser) {
    colorChooser = chooser;
}

void ImageWidget::setCurrentTool(Tool t) {
    currentTool = t;
    updateStatus();
}

void ImageWidget::setBrushSize(int s) {
    brushSize = std::max(1, s);
}



void ImageWidget::undo() {
    if (undoStack.empty()) {
        return;
    }
    
    redoStack.push_back(*image);
    
    if (redoStack.size() > MAX_UNDO) {
        redoStack.erase(redoStack.begin());
    }
    
    *image = undoStack.back();
    undoStack.pop_back();
    
    redraw();
}

void ImageWidget::redo() {
    if (redoStack.empty()) {
        return;
    }
    
    undoStack.push_back(*image);
    *image = redoStack.back();
    redoStack.pop_back();
    
    redraw();
    
}

// experimental
void ImageWidget::updateSize () {
    resize(x(), y(), image->getWidth() * cellSize, image->getHeight() * cellSize);
    redraw();
}


void ImageWidget::flipHorizontal() {
    int w = image->getWidth();
    int h = image->getHeight();
    
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w / 2; x++) {
            Color left = image->getPixel(x, y);
            Color right = image->getPixel(w - x - 1, y);
            
            image->setPixel(x, y, right);
            image->setPixel(w - x - 1, y , left);
        }
    }
    redraw();
}

void ImageWidget::flipVertical() {
    int w = image->getWidth();
    int h = image->getHeight();
    
    for (int y = 0; y < h / 2; y++) {
        for (int x = 0; x < w; x++) {
            Color top = image->getPixel(x, y);
            Color bottom = image->getPixel(x, h - y - 1);
            
            image->setPixel(x, y, bottom);
            image->setPixel(x, h - y - 1, top);

        }
    }
    redraw();
}


void ImageWidget::setCellSize(int size) {
    
    cellSize = std::clamp(size, 2, 128);
    updateSize();
    if(parent()) parent()->redraw();
    updateStatus();
}

int ImageWidget::getCellSize() const {
    return cellSize;
}
Color ImageWidget::getCurrentColor() const {
    return currentColor;
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
    {
        // exprimental
        int midX = image->getWidth() / 2;
        int midY = image->getHeight() / 2;
        
        fl_color (200, 0, 0);
        int thickness = 3;

        if (mirrorHorizontal) {
            fl_rectf(x() + midX * cellSize - thickness/2, 
                     y(),
                     thickness, 
                     image->getHeight() * cellSize
            );
        }

        if (mirrorVertical) {
            fl_rectf(x(), 
                     y() + midY * cellSize - thickness/2,
                     image->getWidth() * cellSize, 
                     thickness
            );
        }
        // ~exprimental
    }

    
    
    
    
    
    if (isDrawingShape && currentTool == TOOL_LINE) {
        fl_color (currentColor.r, currentColor.g, currentColor.b);
        
        
        int x0 = (startX - x()) / cellSize;
        int y0 = (startY - y()) / cellSize;

        int x1 = (previewX - x()) / cellSize;
        int y1 = (previewY - y()) / cellSize;
        
        int dx = std::abs(x1 - x0);
        int dy = std::abs(y1 - y0);
        
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        
        int err = dx - dy;
        
        while(true) {
            drawPixelCell(x0, y0);
            
            
            if (mirrorHorizontal) {
                int mirror_x = image->getWidth() - 1 - x0;
                drawPixelCell(mirror_x, y0);
            }
            if (mirrorVertical) {
                int mirror_y = image->getHeight() - 1 - y0;
                drawPixelCell(x0, mirror_y);
            }
            if (mirrorHorizontal && mirrorVertical) {
                int mirror_x = image->getWidth() - 1 - x0;
                int mirror_y = image->getHeight() - 1 - y0;
                drawPixelCell(mirror_x, mirror_y);
            }
            
            
            if (x0 == x1 && y0 == y1) {
                break;
            }
            
            int e2 = 2 * err;
            
            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }
            
            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
        }
    }
    
    if (isDrawingShape && currentTool == TOOL_CIRCLE) {
        int cx = (startX - x()) / cellSize;
        int cy = (startY - y()) / cellSize;
        
        int px = (previewX - x()) / cellSize;
        int py = (previewY - y()) / cellSize;
        
        int r = std::sqrt(std::pow(px - cx, 2) + std::pow(py - cy, 2));
        
        
        fl_color(currentColor.r, currentColor.g, currentColor.b);
        
        int x0 = 0;
        int y0 = r;
        int d = 1 - r;
        
        while (x0 <= y0) {
            drawCirclePoints(cx, cy, x0, y0);

            if (mirrorHorizontal) {
                int mirror_x = image->getWidth() - 1 - cx;
                drawCirclePoints(mirror_x, cy, x0, y0);
            }

            if (mirrorVertical) {
                int mirror_y = image->getHeight() - 1 - cy;
                drawCirclePoints(cx, mirror_y, x0, y0);
            }

            if (mirrorHorizontal && mirrorVertical) {
                int mirror_x = image->getWidth() - 1 - cx;
                int mirror_y = image->getHeight() - 1 - cy;
                drawCirclePoints(mirror_x, mirror_y, x0, y0);
            }
            
            if (d < 0) {
                d += 2 * x0 + 3;
            }
            else {
                d += 2 * (x0 - y0) + 5;
                y0--;
            }
            x0++;
        }
    }
    
    if (isDrawingShape && currentTool == TOOL_RECTANGLE) {
        int x0 = (startX - x()) / cellSize;
        int y0 = (startY - y()) / cellSize;

        int x1 = (previewX - x()) / cellSize;
        int y1 = (previewY - y()) / cellSize;
        
        fl_color(currentColor.r, currentColor.g, currentColor.b);
        
        int left = std::min(x0, x1);
        int right = std::max(x0, x1);
        int top = std::min(y0, y1);
        int bottom = std::max(y0, y1);
        
        
//        for (int x = left; x <= right; x++) {
//            drawPixelCell(x, y0);
//            drawPixelCell(x, y1);
//        }
//        
//        for (int y = top; y <= bottom; y++) {
//            drawPixelCell(x0, y);
//            drawPixelCell(x1, y);
//        }
        
        
        auto drawRect = [&](int lx, int rx, int ty, int by) {
            for (int x = lx; x <= rx; x++) {
                drawPixelCell(x, ty);
                drawPixelCell(x, by);
            }
            for (int y = ty; y <= by; y++) {
                drawPixelCell(lx, y);
                drawPixelCell(rx, y);
            }
        };
        
        drawRect(left, right, top, bottom);
        
        if (mirrorHorizontal) {
            int mirror_left  = image->getWidth() - 1 - right;
            int mirror_right = image->getWidth() - 1 - left;
            drawRect(mirror_left, mirror_right, top, bottom);
        }
        if (mirrorVertical) {
            int mirror_top    = image->getHeight() - 1 - bottom;
            int mirror_bottom = image->getHeight() - 1 - top;
            drawRect(left, right, mirror_top, mirror_bottom);
        }
        if (mirrorHorizontal && mirrorVertical) {
            int mirror_left  = image->getWidth() - 1 - right;
            int mirror_right = image->getWidth() - 1 - left;
            int mirror_top    = image->getHeight() - 1 - bottom;
            int mirror_bottom = image->getHeight() - 1 - top;
            drawRect(mirror_left, mirror_right, mirror_top, mirror_bottom);
        }
        
        
        
        
        
    }
}

// This function draws cells but it doesn't store the value in the 'pixels' vector 
void ImageWidget::drawPixelCell(int gx, int gy) {
    
    if (gx < 0 || gy < 0) {
        return;
    }
    if (gx >= image->getWidth()) {
        return;
    }
    if (gy >= image->getHeight()) {
        return;
    }
    
    fl_color (currentColor.r, currentColor.g, currentColor.b);
    fl_rectf(
        x() + gx * cellSize,
        y() + gy * cellSize,
        cellSize,
        cellSize
    );
    
    if (showGrid) {
        fl_color(200, 200, 200);
        fl_rect(
            x() + gx * cellSize,
            y() + gy * cellSize,
            cellSize,
            cellSize
        );
    }
}       

// This function actually sets the pixels for the line in the vector
void ImageWidget::setLinePixels(int x0, int y0, int x1, int y1) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    
    int err = dx - dy;
    
    while(true) {
        if (x0 >= 0 && x0 < image->getWidth() &&
            y0 >= 0 && y0 < image->getHeight()) 
            {
                image->setPixel(x0, y0, currentColor);

                if (mirrorHorizontal) {
                    int mirror_x = image->getWidth() - 1 - x0;
                    image->setPixel(mirror_x, y0, currentColor);
                    
                    int px = x() + mirror_x * cellSize;
                    int py = y() + y0 * cellSize;
                    
                    damage(FL_DAMAGE_USER1, px, py, cellSize, cellSize);
                    
                }
                if (mirrorVertical) {
                    int mirror_y = image->getHeight() - 1 - y0;
                    image->setPixel(x0, mirror_y, currentColor);
                    
                    int px = x() + x0 * cellSize;
                    int py = y() + mirror_y * cellSize;
                    
                    damage(FL_DAMAGE_USER1, px, py, cellSize, cellSize);
                    
                }
                if (mirrorHorizontal && mirrorVertical) {
                    int mirror_x = image->getWidth() - 1 - x0;
                    int mirror_y = image->getHeight() - 1 - y0;
                    
                    image->setPixel(mirror_x, mirror_y, currentColor);
                    
                    int px = x() + mirror_x * cellSize;
                    int py = y() + mirror_y * cellSize;
                    
                    damage(FL_DAMAGE_USER1, px, py, cellSize, cellSize);
                }
                
            }
        
        if (x0 == x1 && y0 == y1) {
            break;
        }
        
        int e2 = 2 * err;
        
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void ImageWidget::drawCirclePoints(int cx, int cy, int x, int y) {
    drawPixelCell(cx + x, cy + y);
    drawPixelCell(cx - x, cy + y);
    drawPixelCell(cx + x, cy - y);
    drawPixelCell(cx - x, cy - y);
    
    drawPixelCell(cx + y, cy + x);
    drawPixelCell(cx - y, cy + x);
    drawPixelCell(cx + y, cy - x);
    drawPixelCell(cx - y, cy - x);
}

void ImageWidget::safeSetPixel (int x, int y) {
    
    int w = image->getWidth();
    int h = image->getHeight();
    
    if (x >= 0 && x < w && y >= 0 && y < h) {
        image->setPixel (x, y, currentColor);
    }
    
    
    int px = this->x() + x * cellSize;
    int py = this->y() + y * cellSize;

    damage(FL_DAMAGE_USER1, px, py, cellSize, cellSize);
    
    if (mirrorHorizontal) {
        int mirror_x = w - 1 - x;
        if (mirror_x >= 0 && mirror_x < w && y >= 0 && y < h) {
            image->setPixel(mirror_x, y, currentColor);
        }
        
        int px = this->x() + mirror_x * cellSize;
        int py = this->y() + y * cellSize;
        
        damage(FL_DAMAGE_USER1, px, py, cellSize, cellSize);
        
    }
    if (mirrorVertical) {
        int mirror_y = h - 1 - y;
        if (x >= 0 && x < w && mirror_y >= 0 && mirror_y < h) {
            image->setPixel(x, mirror_y, currentColor);
        }
        
        int px = this->x() + x * cellSize;
        int py = this->y() + mirror_y * cellSize;
        
        damage(FL_DAMAGE_USER1, px, py, cellSize, cellSize);
        
    }
    if (mirrorHorizontal && mirrorVertical) {
        int mirror_x = w - 1 - x;
        int mirror_y = h - 1 - y;
        
        if (mirror_x >= 0 && mirror_x < w && mirror_y >= 0 && mirror_y < h) {
            image->setPixel(mirror_x, mirror_y, currentColor);
        }
        
        
        int px = this->x() + mirror_x * cellSize;
        int py = this->y() + mirror_y * cellSize;
        
        damage(FL_DAMAGE_USER1, px, py, cellSize, cellSize);
    }
}

void ImageWidget::set8CirclePixels (int cx, int cy, int x, int y) {
    safeSetPixel(cx + x, cy + y);
    safeSetPixel(cx - x, cy + y);
    safeSetPixel(cx + x, cy - y);
    safeSetPixel(cx - x, cy - y);

    safeSetPixel(cx + y, cy + x);
    safeSetPixel(cx - y, cy + x);
    safeSetPixel(cx + y, cy - x);
    safeSetPixel(cx - y, cy - x);
}


// This function actually sets the pixels for the circle in the vector
void ImageWidget::setCirclePixels (int cx, int cy, int r) {
    int x0 = 0;
    int y0 = r;
    int d = 1 - r;
    
    while (x0 <= y0) {
        set8CirclePixels (cx, cy, x0, y0);
        
        if (d < 0) {
            d += 2 * x0 + 3;
        }
        else {
            d += 2 * (x0 - y0) + 5;
            y0--;
        }
        x0++;
    }
}

void ImageWidget::pickColorAtMouse(int mouseX, int mouseY) {
    int gridX = (mouseX - x()) / cellSize;
    int gridY = (mouseY - y()) / cellSize;
    
    if (gridX >= 0 && gridX < image->getWidth() &&
        gridY >= 0 && gridY < image->getHeight()) {

        Color c = image->getPixel(gridX, gridY);
        setCurrentColor(c);
    }
    
}

void ImageWidget::drawAtMouse(int mouseX, int mouseY) {
    int X = (mouseX - x()) / cellSize;
    int Y = (mouseY - y()) / cellSize;
    
    int r = brushSize / 2;
    for (int j = -r; j <= r; j++) {
        for (int i = -r; i <= r; i++) {
            
            int gridX = X + i;
            int gridY = Y + j;
            
            
            
            
            if (gridX >= 0 && gridX < image->getWidth() && 
                gridY >= 0 && gridY < image->getHeight()) {
                    
                    image->setPixel(gridX, gridY, currentColor);

                    
                    if (mirrorHorizontal) {
                        int mirror_x = image->getWidth() - 1 - gridX;
                        image->setPixel(mirror_x, gridY, currentColor);
                        
                        int px = x() + mirror_x * cellSize;
                        int py = y() + gridY * cellSize;
                        
                        damage(FL_DAMAGE_USER1, px, py, cellSize, cellSize);
                        
                    }
                    if (mirrorVertical) {
                        int mirror_y = image->getHeight() - 1 - gridY;
                        image->setPixel(gridX, mirror_y, currentColor);
                        
                        int px = x() + gridX * cellSize;
                        int py = y() + mirror_y * cellSize;
                        
                        damage(FL_DAMAGE_USER1, px, py, cellSize, cellSize);
                        
                    }
                    if (mirrorHorizontal && mirrorVertical) {
                        int mirror_x = image->getWidth() - 1 - gridX;
                        int mirror_y = image->getHeight() - 1 - gridY;
                        
                        image->setPixel(mirror_x, mirror_y, currentColor);
                        
                        int px = x() + mirror_x * cellSize;
                        int py = y() + mirror_y * cellSize;
                        
                        damage(FL_DAMAGE_USER1, px, py, cellSize, cellSize);
                    }
                         
                    int px = x() + gridX * cellSize;
                    int py = y() + gridY * cellSize;
                    
                    damage(FL_DAMAGE_USER1, px, py, cellSize, cellSize);
            }
            
        }
    }
    
    
}

void ImageWidget::floodFill(int startX, int startY) {
    Color target = image->getPixel (startX, startY);
    if (target == currentColor) {
        return;
    }
    
    std::queue<std::pair<int, int>> q;
    q.push({startX, startY});
    
    while (!q.empty()) {
        auto p = q.front();
        q.pop();
        
        int x = p.first;
        int y = p.second;
        
        if (x < 0 || y < 0) {
            continue;
        }
        if (x >= image->getWidth()) {
            continue;
        }
        if (y >= image->getHeight()) {
            continue;
        }

        if (!(image->getPixel(x, y) == target)) {
            continue;
        }
        
        image->setPixel(x, y, currentColor);
        
        q.push({x + 1, y});
        q.push({x - 1, y});
        q.push({x, y + 1});
        q.push({x, y - 1});
    }
}









int ImageWidget::handle (int event) {
    int button = Fl::event_button();
    switch (event) {
        case FL_PUSH: {
            
            if (button == FL_MIDDLE_MOUSE) {
                middleDrag = true;
                dragLastX = Fl::event_x();
                dragLastY = Fl::event_y();
                fl_cursor(FL_CURSOR_HAND);
                return 1;
            }
            
            
            
            
            
            if (Fl::event_alt()) {
                pickColorAtMouse(Fl::event_x(), Fl::event_y());
                return 1;
            }
            
            if (button != FL_LEFT_MOUSE) {
                return 1;
            }
            
            
            if (currentTool != TOOL_PEN) {
                previewX = startX = Fl::event_x();
                previewY = startY = Fl::event_y();
                isDrawingShape = true;
                return 1;
            }
            
            
            undoStack.push_back(*image);
            
            if (undoStack.size() > MAX_UNDO) {
                undoStack.erase(undoStack.begin());
            }
            
            redoStack.clear();

            drawAtMouse(Fl::event_x(), Fl::event_y());
            
            return 1;
        }
        case FL_DRAG: {
            updateStatus();
            if (middleDrag) {
                int dx = Fl::event_x() - dragLastX;
                int dy = Fl::event_y() - dragLastY;
                
                if (Fl_Scroll *scroll = dynamic_cast<Fl_Scroll *>(parent())) {
                    scroll->scroll_to(
                        scroll->xposition() - dx,
                        scroll->yposition() - dy
                    );
                }
                
                dragLastX = Fl::event_x();
                dragLastY = Fl::event_y();
                
            }
            
            if (Fl::event_alt()) {
                pickColorAtMouse(Fl::event_x(), Fl::event_y());
                return 1;
            }
            
            if (button != FL_LEFT_MOUSE) {
                return 1;
            }
            
            
            if (isDrawingShape && currentTool != TOOL_PEN) {
                previewX = Fl::event_x();
                previewY = Fl::event_y();
                
                redraw();
                return 1;
            }
            
            
            
            
            drawAtMouse(Fl::event_x(), Fl::event_y());
            return 1;
        }
        case FL_RELEASE: {
            
            if (Fl::event_button() == FL_MIDDLE_MOUSE) {
                middleDrag = false;
                fl_cursor(FL_CURSOR_DEFAULT);
                return 1;
            }
            
            if (button != FL_LEFT_MOUSE) {
                return 1;
            }
            
            
            
            if (isDrawingShape && currentTool == TOOL_LINE) {
                endX = Fl::event_x();
                endY = Fl::event_y();
                
                int x0 = (startX - x()) / cellSize;
                int y0 = (startY - y()) / cellSize;
                
                int x1 = (endX - x()) / cellSize;
                int y1 = (endY - y()) / cellSize;
                
                undoStack.push_back(*image);
                redoStack.clear();
                
                setLinePixels(x0, y0, x1, y1);
                
//                redraw();
                
            }
            else if (isDrawingShape && currentTool == TOOL_CIRCLE) {
                int cx = (startX - x()) / cellSize;
                int cy = (startY - y()) / cellSize;

                int px = (Fl::event_x() - x()) / cellSize;
                int py = (Fl::event_y() - y()) / cellSize;

                int r = std::sqrt(std::pow(px - cx, 2) + std::pow(py - cy, 2));

                undoStack.push_back(*image);
                redoStack.clear();

                setCirclePixels(cx, cy, r);
            }
            else if (isDrawingShape && currentTool == TOOL_BUCKET) {
                int px = (Fl::event_x() - x()) / cellSize;
                int py = (Fl::event_y() - y()) / cellSize;
                
                
                undoStack.push_back(*image);
                redoStack.clear();
                
                floodFill(px, py);
                redraw();
            }
            else if (isDrawingShape && currentTool == TOOL_RECTANGLE) {
                int x0 = (startX - x()) / cellSize;
                int y0 = (startY - y()) / cellSize;

                int x1 = (Fl::event_x() - x()) / cellSize;
                int y1 = (Fl::event_y() - y()) / cellSize;
                
                
                int left = std::min(x0, x1);
                int right = std::max(x0, x1);
                int top = std::min(y0, y1);
                int bottom = std::max(y0, y1);
                
                undoStack.push_back(*image);
                redoStack.clear();
                
                
                for (int x = left; x <= right; x++) {
                    safeSetPixel(x, y0);
                    safeSetPixel(x, y1);
                }

                for (int y = top; y <= bottom; y++) {
                    safeSetPixel(x0, y);
                    safeSetPixel(x1, y);
                }
            }
            isDrawingShape = false;
            return 1;
        }
        case FL_ENTER: {
            updateStatus();
            return 1;
        }
        case FL_LEAVE: {
            updateStatus();
            return 1;
        }
        case FL_MOVE: {
            updateStatus();
            return 1;
        }
        
        
        case FL_MOUSEWHEEL: {
            updateStatus();
            double dy = Fl::event_dy();
//            if (dy < 0) {
//                setCellSize(cellSize + 2);
//            }
//            else {
//                setCellSize(cellSize - 2);
//            }
            
            if (dy == 0) return 0;
            
            int mx = Fl::event_x();
            int my = Fl::event_y();
            
            double imgX = (mx - x()) / (double)cellSize;
            double imgY = (my - y()) / (double)cellSize;
            
            int oldCellSize = cellSize;
            int delta = (dy < 0) ? 1 : -1;
            setCellSize(cellSize + delta);
            
            
            
            
            
            
            
            if (cellSize == oldCellSize) return 1;
            
            double newImgX = (mx - x()) / (double)cellSize;
            double newImgY = (my - y()) / (double)cellSize;
            
            double dx = imgX - newImgX;
            dy = imgY - newImgY;
            
            int scrollDx = (int)std::round(dx * cellSize);
            int scrollDy = (int)std::round(dy * cellSize);
            
            if (Fl_Scroll *scroll = dynamic_cast<Fl_Scroll *>(parent())) {
                scroll->scroll_to(
                    scroll->xposition() + scrollDx,
                    scroll->yposition() + scrollDy
                );
            }
            
            redraw();
            return 1;
        }
    }
    return Fl_Widget::handle(event);
}
Image* ImageWidget::getImage() {
    return image;
}


void ImageWidget::updateStatus() {
    if (!window() || window()->children() == 0) {
        return;
    }
    
    Fl_Widget *last = window()->child(window()->children() - 1);
    Fl_Flex *flex = dynamic_cast<Fl_Flex *>(last);
    if (!flex || flex->children() == 0) {
        return;
    }
    
    Fl_Box *box = dynamic_cast<Fl_Box *>(flex->child(0));
    if (!box) {
        return;
    }
    
    int zoom = (int)std::round((cellSize / 20.0) * 100.0);
    
    
    int mx = -1;
    int my = -1;
    
    if (Fl::event_inside(this)) {
        int local_x = Fl::event_x() - x();
        int local_y = Fl::event_y() - y();
        
        mx = local_x / cellSize;
        my = local_y / cellSize;
        
        if (mx < 0 || mx >= image->getWidth()) {
            mx = -1;
        }
        if (my < 0 || my >= image->getHeight()) {
            my = -1;
        }
        
    }
    
    
    const char *toolName = "?";
    
    switch (currentTool) {
        case TOOL_PEN:
            toolName = "Pen"; break;
        case TOOL_LINE:
            toolName = "Line"; break;
        case TOOL_CIRCLE:
            toolName = "Circle"; break;
        case TOOL_RECTANGLE:
            toolName = "Rectangle"; break;
        case TOOL_BUCKET:
            toolName = "Bucket"; break;
    }
    
    
    char buffer[160];
    
    snprintf(
        buffer, sizeof(buffer), 
        "Zoom: %d%% | X:%d Y:%d | %dx%d px | %s",
        zoom, mx + 1, my + 1, image->getWidth(), image->getHeight(),
        toolName
    );
    
    box->copy_label(buffer);
    box->redraw();
    flex->redraw();
    
}








struct uiData {
    ImageWidget *widget;
    Fl_Box *preview;
    HoverLightButton *penButton;
};

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

void OnGridToggle (Fl_Widget *w, void *data) {
    ImageWidget* widget = (ImageWidget*)data;
    Fl_Menu_Bar* menubar = (Fl_Menu_Bar*)w;

    const Fl_Menu_Item* gridItem = menubar->find_item("Edit/Show Grid");
    widget->setShowGrid(gridItem->value());
}

void OnHorizontalMirror (Fl_Widget *w, void *data) {
    ImageWidget* widget = (ImageWidget*)data;
    Fl_Menu_Bar* menubar = (Fl_Menu_Bar*)w;
    
    const Fl_Menu_Item* gridItem = menubar->find_item("Edit/Horizontal Mirror");
    widget->setMirrorHorizontal(gridItem->value());
    widget->redraw();
}
void OnVerticalMirror (Fl_Widget *w, void *data) {
    ImageWidget* widget = (ImageWidget*)data;
    Fl_Menu_Bar* menubar = (Fl_Menu_Bar*)w;
    
    const Fl_Menu_Item* gridItem = menubar->find_item("Edit/Vertical Mirror");
    widget->setMirrorVertical(gridItem->value());
    widget->redraw();
}

void OnFlipHorizontal (Fl_Widget *w, void *data) {
    ImageWidget *widget = (ImageWidget *)data;
    widget->flipHorizontal();
}

void OnFlipVertical (Fl_Widget *w, void *data) {
    ImageWidget *widget = (ImageWidget *)data;
    widget->flipVertical();
}


void OnToolSelect (Fl_Widget *w, void *data) {
    ImageWidget *widget = (ImageWidget *)data;
    Fl_Light_Button *btn = (Fl_Light_Button *)w;
    
    if (!btn->value()) {
        return;
    }
    
    if (btn->label() == std::string("Pen")) {
        widget->setCurrentTool(ImageWidget::TOOL_PEN);
    }
    else if (btn->label() == std::string("Line")) {
        widget->setCurrentTool(ImageWidget::TOOL_LINE);
    }
    else if (btn->label() == std::string("Circle")) {
        widget->setCurrentTool(ImageWidget::TOOL_CIRCLE);
    }
    else if (btn->label() == std::string("Bucket")) {
        widget->setCurrentTool(ImageWidget::TOOL_BUCKET);
    }
    else if (btn->label() == std::string("Rectangle")) {
        widget->setCurrentTool(ImageWidget::TOOL_RECTANGLE);
    }
}


void Oneraser (Fl_Widget *w, void *data) {
    uiData *ui = (uiData*)data;
    ui->widget->setCurrentColor({255, 255, 255});
    ui->widget->setCurrentTool(ImageWidget::TOOL_PEN);
    if (ui->penButton) {
        ui->penButton->setonly();
        ui->penButton->redraw();
    }
}

void OnPickColor (Fl_Widget *w, void *data) {
    uiData *ui = (uiData*)data;
    
    Color c = ui->widget->getCurrentColor();
    
    
    Fl_Color _c = fl_rgb_color (c.r, c.g, c.b);
    _c = fl_show_colormap(_c);
    
    c = convertColor(_c);
    
    ui->widget->setCurrentColor(c);

}

void OnBrushSlider (Fl_Widget *w, void *data) {
    Fl_Value_Slider *slider = (Fl_Value_Slider *)w;
    ImageWidget *widget = (ImageWidget *)data;
    
    double value = slider->value();
    widget->setBrushSize(value);
}

void OnClear (Fl_Widget *w, void *data) {
    int r = fl_choice(
                      "Clear the entire canvas?",
                      "Cancel",
                      "Yes",
                      0
    );
    
    if (r == 1) {
        
    }
    
}

void OnUndo (Fl_Widget *w, void *data) {
    ImageWidget *widget = (ImageWidget *)data;
    widget->undo();
}
void OnRedo (Fl_Widget *w, void *data) {
    ImageWidget *widget = (ImageWidget *)data;
    widget->redo();
}

void OnAbout(Fl_Widget *w, void *data) {
    fl_message(
        "AnotherPixEditor\n"
        "  2026 M.H.Jim\n"
        "Built with FLTK 1.4.4"
    );
}





int main (int argc, char ** argv) {
    Fl::scheme("gleam");
    
    
    
 
    
    
    float r = 0.22f;
    float g = 0.24f;
    float b = 0.26f;
    
    Fl_Double_Window *window = new Fl_Double_Window(1200, 800, "AnotherPixEditor");
    window->color(fl_rgb_color(46, 53, 61));
    
    
    // menu bar
//    Fl_Menu_Bar *menubar = new Fl_Menu_Bar(0, 0, 1200, 30);
    HoverMenu *menubar = new HoverMenu(0, 0, 1200, 30);
    
    menubar->box(FL_THIN_DOWN_BOX);
    menubar->color(fl_rgb_color(45, 50, 58));
    menubar->textcolor(FL_WHITE);
    
    
    
    
    
    // menu bar end
    Image *img = new Image(32, 32);
    
    //----------------------------------------------------------------------------------------------
    
    
    Fl_Group *toolbar = new Fl_Group(0, 30, 220, 770);
    toolbar->box(FL_FLAT_BOX);
    toolbar->color(fl_rgb_color(36, 40, 47));
    

    Fl_Pack *tools = new Fl_Pack(15, 100, 190, 770);
    tools->type(Fl_Flex::VERTICAL);
    tools->spacing(10);
    
    HoverButton *eraser = new HoverButton (0, 0, 0, 30, "Eraser");
    HoverLightButton *penTool    = new HoverLightButton(0, 0, 0, 30, "Pen");
    HoverLightButton *lineTool   = new HoverLightButton(0, 0, 0, 30, "Line");
    HoverLightButton *rectangleTool = new HoverLightButton(0, 0, 0, 30, "Rectangle");
    HoverLightButton *circleTool = new HoverLightButton(0, 0, 0, 30, "Circle");
    HoverLightButton *bucketTool = new HoverLightButton(0, 0, 0, 30, "Bucket");
//    HoverButton *pickColor = new HoverButton (0, 0, 0, 30, "pick color"); 
    
    
    

    
    
    

//    tools->fixed(eraser, 30);
//    tools->fixed(penTool, 30);
//    tools->fixed(lineTool, 30);
//    tools->fixed(circleTool, 30);
//    tools->fixed(bucketTool, 30);
//    tools->fixed(pickColor, 30);
//    tools->fixed(preview, 30);

    
    penTool->type(FL_RADIO_BUTTON);
    lineTool->type(FL_RADIO_BUTTON);
    rectangleTool->type(FL_RADIO_BUTTON);
    circleTool->type(FL_RADIO_BUTTON);
    bucketTool->type(FL_RADIO_BUTTON);
    
    penTool->setonly();
    
    
    Fl_Box *preview = new Fl_Box (0, 0, 0, 30, "Color");
    preview->color(FL_BLACK);
    
    
    Fl_Value_Slider *brushSlider = new Fl_Value_Slider (0, 0, 0, 30);
    brushSlider->type(FL_HOR_FILL_SLIDER);
    brushSlider->bounds(1, 10);
    brushSlider->value(1);
    brushSlider->step(1);
    brushSlider->textcolor(FL_WHITE);
    brushSlider->color(fl_rgb_color(55,60,70));
    
    
    
    
    
    eraser->box(FL_SHADOW_BOX);
    penTool->box(FL_SHADOW_BOX);
    lineTool->box(FL_SHADOW_BOX);
    rectangleTool->box(FL_SHADOW_BOX);
    circleTool->box(FL_SHADOW_BOX);
    bucketTool->box(FL_SHADOW_BOX);
//    pickColor->box(FL_SHADOW_BOX);
    preview->box(FL_EMBOSSED_BOX);
    
    
    
    
    eraser->align(FL_ALIGN_CENTER);
    penTool->align(FL_ALIGN_CENTER);
    lineTool->align(FL_ALIGN_CENTER);
    rectangleTool->align(FL_ALIGN_CENTER);
    circleTool->align(FL_ALIGN_CENTER);
    bucketTool->align(FL_ALIGN_CENTER);
//    pickColor->align(FL_ALIGN_CENTER);
    preview->align(FL_ALIGN_CENTER);
    
    eraser->color(fl_rgb_color(92, 103, 117));
    penTool->color(fl_rgb_color(92, 103, 117));
    lineTool->color(fl_rgb_color(92, 103, 117));
    rectangleTool->color(fl_rgb_color(92, 103, 117));
    circleTool->color(fl_rgb_color(92, 103, 117));
    bucketTool->color(fl_rgb_color(92, 103, 117));
//    pickColor->color(fl_rgb_color(92, 103, 117));
    
    eraser->selection_color(fl_rgb_color(100, 160, 255));
    penTool->selection_color(fl_rgb_color(100, 160, 255));
    lineTool->selection_color(fl_rgb_color(100, 160, 255));
    rectangleTool->selection_color(fl_rgb_color(100, 160, 255));
    circleTool->selection_color(fl_rgb_color(100, 160, 255));
    bucketTool->selection_color(fl_rgb_color(100, 160, 255));
//    pickColor->selection_color(fl_rgb_color(150, 170, 200));
    
    
    
    
    Fl_Color_Chooser *color_chooser = new Fl_Color_Chooser(0, 0, 0, 100);
    color_chooser->box(FL_NO_BOX);
    color_chooser->color(fl_rgb_color(45, 50, 58));
    color_chooser->selection_color(fl_rgb_color(90, 150, 220));
    
    
    
    
    
    
    
    
    
    
    
    tools->end();
    
    toolbar->resizable(NULL);
    toolbar->end();
    
    //----------------------------------------------------------------------------------------------

    
    
    
   
    Fl_Scroll *scroll = new Fl_Scroll(240, 50, 960, 700);
    scroll->box(FL_NO_BOX);
    scroll->color(fl_rgb_color(26, 30, 38));

    
    ImageWidget *widget = new ImageWidget(280, 50,
                                       img->getWidth() * 20,
                                       img->getHeight() * 20,
                                       img);
    widget->setPreviewBox(preview);
    widget->setColorChooser(color_chooser);
    
    scroll->add(widget);
    scroll->end();


    
    
    
    
    
    
    
    
    uiData *ui = new uiData {widget, preview, penTool};
    
    
    
    eraser->callback(Oneraser, ui);
//    pickColor->callback(OnPickColor, ui);
    
    
    penTool->callback(OnToolSelect, widget);
    lineTool->callback(OnToolSelect, widget);
    rectangleTool->callback(OnToolSelect, widget);
    circleTool->callback(OnToolSelect, widget);
    bucketTool->callback(OnToolSelect, widget);
    brushSlider->callback(OnBrushSlider, widget);
    
    
    
    
    
    
    
    color_chooser->callback([](Fl_Widget *w, void *data) {
        Fl_Color_Chooser *chooser = static_cast<Fl_Color_Chooser *>(w);
        ImageWidget *widget = static_cast<ImageWidget *>(data);
        
        double rr = chooser->r();
        double gg = chooser->g();
        double bb = chooser->b();
            
            
        Color c;
        c.r = (int) (rr * 255);
        c.g = (int) (gg * 255);
        c.b = (int) (bb * 255);
        
        widget->setCurrentColor(c);
    
    }, widget);
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    menubar->add("File/Open" ,FL_CTRL + 'o', load, widget);
    menubar->add("File/Save", FL_CTRL + 's', save, widget);
    
    menubar->add("Edit/Undo", FL_CTRL + 'z', OnUndo, widget);
    menubar->add("Edit/Redo", FL_CTRL + 'y', OnRedo, widget);
    menubar->add("Edit/Flip Horizontally", 0, OnFlipHorizontal, widget);
    menubar->add("Edit/Flip Vertically", 0, OnFlipVertical, widget);
    menubar->add("Edit/Show Grid", 'g', OnGridToggle, widget, FL_MENU_TOGGLE);
    menubar->add("Edit/Horizontal Mirror", FL_CTRL + FL_SHIFT + 'h', OnHorizontalMirror, widget, FL_MENU_TOGGLE);
    menubar->add("Edit/Vertical Mirror", FL_CTRL + FL_SHIFT + 'v', OnVerticalMirror, widget, FL_MENU_TOGGLE);
    
    
    menubar->add("Color", 0, OnPickColor, ui);
    menubar->add("About", 0, OnAbout);
    
    
    
    
    
    
    
    
    
    
    
    //experimental
    const int STATUS_HEIGHT = 30;
    
    Fl_Flex *flex = new Fl_Flex(0, window->h() - STATUS_HEIGHT, window->w(), STATUS_HEIGHT);
    flex->type(Fl_Flex::VERTICAL);
    flex->begin();


    Fl_Box *statusbar = new Fl_Box(0, 0, 0, STATUS_HEIGHT);
    statusbar->box(FL_FLAT_BOX);
    statusbar->color(fl_rgb_color(40, 44, 52));
    statusbar->labelcolor(FL_WHITE);
    statusbar->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    statusbar->label("Zoom: 100% | X:0 Y:0 | 32x32 px | Pen");

    flex->end();
    
    
    
    window->add(flex);
    
    window->resizable(scroll);

    window->end();
    window->show (argc, argv);
    
    
    
    return Fl::run();
}
