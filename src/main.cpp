#include <vector>
#include <cmath>
#include <queue>
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
#include <FL/fl_show_colormap.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Value_Slider.H>


const int MAX_UNDO = 50;




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

        
        Tool currentTool;
        int startX;
        int startY;
        int endX;
        int endY;
        int previewX;
        int previewY;
        bool isDrawingShape;
        
        
        
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
                brushSize = 2;
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

void ImageWidget::setCurrentTool(Tool t) {
    currentTool = t;
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
        
        
        for (int x = left; x <= right; x++) {
            drawPixelCell(x, y0);
            drawPixelCell(x, y1);
        }
        
        for (int y = top; y <= bottom; y++) {
            drawPixelCell(x0, y);
            drawPixelCell(x1, y);
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
    switch (event) {
        case FL_PUSH: {
            
            if (Fl::event_alt()) {
                pickColorAtMouse(Fl::event_x(), Fl::event_y());
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
            
            if (Fl::event_alt()) {
                pickColorAtMouse(Fl::event_x(), Fl::event_y());
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
}

void OnPickColor (Fl_Widget *w, void *data) {
    uiData *ui = (uiData*)data;
    
    Color c = ui->widget->getCurrentColor();
    
    
    Fl_Color _c = fl_rgb_color (c.r, c.g, c.b);
    _c = fl_show_colormap(_c);
    
    c = convertColor(_c);
    
    ui->widget->setCurrentColor(c);

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
        "© 2026 M.H.Jim\n"
        "Built with FLTK 1.4.4"
    );
}


int main (int argc, char ** argv) {
    Fl::scheme("gleam");
    
    float r = 0.5f;
    float g = 0.5f;
    float b = 0.5f;
    
    Fl_Double_Window *window = new Fl_Double_Window(1200, 800, "AnotherPixEditor");
    window->color(fl_rgb_color(
        (int)(r * 255),
        (int)(g * 255),
        (int)(b * 255)
    ));
    
    
    // menu bar
    Fl_Menu_Bar *menubar = new Fl_Menu_Bar(0, 0, 1200, 30);
    
    
    Image *img = new Image(32, 32);
    
    
    
    
    
    
    Fl_Group *leftPanel = new Fl_Group(0, 30, 250, 770);
    leftPanel->box(FL_BORDER_BOX);
    leftPanel->resizable(0);
    //----------------------------------------------------------------------------------------------
    Fl_Button *eraser = new Fl_Button (10, 70, 100, 30, "Eraser");
    
    Fl_Box *preview = new Fl_Box (70, 110, 120, 25);
    preview->box (FL_BORDER_BOX);
    preview->color (fl_rgb_color(0, 0, 0));
    
    Fl_Box *previewColorLabel = new Fl_Box (10, 110, 60, 25, "Color:");
    previewColorLabel->labelfont (FL_BOLD);
    
    Fl_Button *pickColor = new Fl_Button (10, 150, 100, 30, "pick color"); 
    
    
    
    
    
    
    Fl_Light_Button *penTool    = new Fl_Light_Button(10, 190, 100, 30, "Pen");
    Fl_Light_Button *lineTool   = new Fl_Light_Button(10, 230, 100, 30, "Line");
    Fl_Light_Button *circleTool = new Fl_Light_Button(10, 270, 100, 30, "Circle");
    Fl_Light_Button *bucketTool = new Fl_Light_Button(10, 310, 100, 30, "Bucket");
    Fl_Light_Button *rectangleTool = new Fl_Light_Button(10, 350, 100, 30, "Rectangle");
    
    penTool->type(FL_RADIO_BUTTON);
    lineTool->type(FL_RADIO_BUTTON);
    circleTool->type(FL_RADIO_BUTTON);
    bucketTool->type(FL_RADIO_BUTTON);
    rectangleTool->type(FL_RADIO_BUTTON);
    
    penTool->setonly();
    
    
    
    Fl_Value_Slider *brushSlider = new Fl_Value_Slider (10, 390, 100, 30, "Slider");
    brushSlider->type(FL_HOR_NICE_SLIDER);
    brushSlider->bounds(0, 100);
    brushSlider->value(50);
    brushSlider->step(1);
    
    
    
    
    
    
    //----------------------------------------------------------------------------------------------
    leftPanel->end();
    
   
   
   
   
    Fl_Scroll *scroll = new Fl_Scroll(280, 50, 2 * img->getWidth() * 20, 2 * img->getHeight() * 20);


    scroll->color(fl_rgb_color(
        (int)(r * 255),
        (int)(g * 255),
        (int)(b * 255)
    ));
    
    ImageWidget *widget = new ImageWidget(280, 50,
                                       img->getWidth() * 20,
                                       img->getHeight() * 20,
                                       img);
    widget->setPreviewBox(preview);
    
    scroll->end();
    
    uiData *ui = new uiData {widget, preview};
    
    
    
    eraser->callback(Oneraser, ui);
    pickColor->callback(OnPickColor, ui);
    
    
    penTool->callback(OnToolSelect, widget);
    lineTool->callback(OnToolSelect, widget);
    circleTool->callback(OnToolSelect, widget);
    bucketTool->callback(OnToolSelect, widget);
    rectangleTool->callback(OnToolSelect, widget);
    
    
    
    menubar->add("File/Open" ,FL_CTRL + 'o', load, widget);
    menubar->add("File/Save", FL_CTRL + 's', save, widget);
    
    menubar->add("Edit/Undo", FL_CTRL + 'z', OnUndo, widget);
    menubar->add("Edit/Redo", FL_CTRL + 'y', OnRedo, widget);
    menubar->add("Edit/Flip Horizontally", 0, OnFlipHorizontal, widget);
    menubar->add("Edit/Flip Vertically", 0, OnFlipVertical, widget);
    menubar->add("Edit/Show Grid", 'g', OnGridToggle, widget, FL_MENU_TOGGLE);
    menubar->add("Edit/Horizontal Mirror", FL_CTRL + FL_SHIFT + 'h', OnHorizontalMirror, widget, FL_MENU_TOGGLE);
    menubar->add("Edit/Vertical Mirror", FL_CTRL + FL_SHIFT + 'v', OnVerticalMirror, widget, FL_MENU_TOGGLE);
    
    
    menubar->add("Color", 0, chooseColor, ui);
    menubar->add("About", 0, OnAbout);
    
    
    
    
    window->resizable(scroll);
    window->end();
    window->show (argc, argv);
    
    return Fl::run();
}
