#include "text.h"

static bool mapCharTextureToMesh(FontBitmap& font_bitmap, const char c, glm::vec2* start_texture_map, uint16_t texture_map_stride);
static void printGlyphInformation(FT_GlyphSlot glyph);
static void createRectMesh(uint16_t * indices, glm::vec2 * vertices, uint16_t vertices_stride_bytes, uint16_t start_vertex_index, float x, float y, float height, float width);

inline double signedNormalizePixelPosition(uint32_t position_pixels, uint32_t length_pixels)
{
    assert(position_pixels <= length_pixels);
    assert(length_pixels != 0);

    if(position_pixels > length_pixels) {
        return 1.0;
    }

    int sign = 1;

    if(position_pixels < length_pixels / 2) {
        sign = -1;
        position_pixels = length_pixels - position_pixels;
    }

    return (static_cast<double>(position_pixels) / length_pixels) * sign;
}

inline double unsignedNormalizePixelPosition(uint32_t position_pixels, uint32_t length_pixels)
{
    assert(position_pixels <= length_pixels);
    assert(length_pixels != 0);

    return static_cast<double>(position_pixels) / length_pixels;
}

static bool mapCharTextureToMesh(FontBitmap& font_bitmap, const char c, glm::vec2* start_texture_map, uint16_t texture_map_stride)
{
    float x_pos_norm = std::get<1>(font_bitmap.char_data[c]).x;
    float y_pos_norm = std::get<1>(font_bitmap.char_data[c]).y;
    float width_norm = static_cast<float>(unsignedNormalizePixelPosition(std::get<0>(font_bitmap.char_data[c]).width, font_bitmap.texture_width));
    float height_norm = static_cast<float>(unsignedNormalizePixelPosition(std::get<0>(font_bitmap.char_data[c]).height, font_bitmap.texture_height));

//    printf("Vertex texture coord stride -> %u\n", texture_map_stride);

    uint8_t * byte_pos = reinterpret_cast<uint8_t *>(start_texture_map);

    *reinterpret_cast<glm::vec2 *>(byte_pos) =                              {x_pos_norm, y_pos_norm + height_norm};                 // Top, left
    *reinterpret_cast<glm::vec2 *>(byte_pos + (texture_map_stride * 1)) =   {x_pos_norm + width_norm, y_pos_norm + height_norm};    // Top, right
    *reinterpret_cast<glm::vec2 *>(byte_pos + (texture_map_stride * 2)) =   {x_pos_norm + width_norm, y_pos_norm};                  // Bottom, right
    *reinterpret_cast<glm::vec2 *>(byte_pos + (texture_map_stride * 3)) =   {x_pos_norm, y_pos_norm};                               // Bottom, left

//    printf("Mapping '%c' with following dimensions ->\n{%f, %f}{%f, %f}\n{%f, %f}{%f, %f}\n", c,
//                                        x_pos_norm, y_pos_norm + height_norm, x_pos_norm + width_norm, y_pos_norm + height_norm,
//                                        x_pos_norm, y_pos_norm, x_pos_norm + width_norm, y_pos_norm);

    return true;
}

/*bool setupBitmapForCharacterSet( FontBitmap& font_bitmap,
                                uint16_t texture_width_cells,
                                uint16_t cell_size,
                                uint16_t num_charactors)*/

bool instanciateFontBitmap(FontBitmap& outFontBitmap, FT_Face& face, const char * uniqueCharsString, uint16_t textureWidthCells, uint16_t cellSize )
{
    if(! setupBitmapForCharacterSet(outFontBitmap, textureWidthCells, cellSize, strlen(uniqueCharsString)) ) {
        return false;
    }

    while(*uniqueCharsString != '\0')
    {
        if(! FontBitmap::instanciate_char_bitmap(outFontBitmap, face, *(uniqueCharsString)) ) {
            return false;
        }

        uniqueCharsString++;
    }

    return true;
}

bool FontBitmap::instanciate_char_bitmap(FontBitmap& font_bitmap, FT_Face& face, const char c)
{
    if(font_bitmap.char_data.count(c) != 0) {
        puts("Charactor already found in bitmap");
        return false;
    }

    if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
        puts("Failed to load charactor");
        return false;
    }

    uint16_t bitmap_width = face->glyph->bitmap.width;
    uint16_t bitmap_height = face->glyph->bitmap.rows;

    assert(font_bitmap.cell_size != 0);

    uint16_t width_cells = font_bitmap.texture_width / font_bitmap.cell_size;

    if(font_bitmap.current_x_cell >= width_cells) {
        font_bitmap.current_x_cell = 0;
        font_bitmap.current_y_cell++;
    }

    uint16_t x_pixel_pos = font_bitmap.current_x_cell * font_bitmap.cell_size;
    uint16_t y_pixel_pos = font_bitmap.current_y_cell * font_bitmap.cell_size;

    RGBA_8UNORM * current_pixel = font_bitmap.bitmap_data + (font_bitmap.texture_width * y_pixel_pos) + x_pixel_pos;

    assert(font_bitmap.texture_width > bitmap_width);

    uint16_t texture_width = font_bitmap.texture_width;
    uint16_t src_y_index;

    for(uint16_t y = 0; y < bitmap_height; y++)
    {
        src_y_index = bitmap_height - y - 1; // y is flipped for loaded bitmap

        for(uint16_t x = 0; x < bitmap_width; x++)
        {
            (current_pixel + (y * texture_width) + x)->r = *(face->glyph->bitmap.buffer + (src_y_index * bitmap_width) + x);
            (current_pixel + (y * texture_width) + x)->g = *(face->glyph->bitmap.buffer + (src_y_index * bitmap_width) + x);
            (current_pixel + (y * texture_width) + x)->b = *(face->glyph->bitmap.buffer + (src_y_index * bitmap_width) + x);
            (current_pixel + (y * texture_width) + x)->a = *(face->glyph->bitmap.buffer + (src_y_index * bitmap_width) + x);
        }
    }

    assert(font_bitmap.texture_width != 0);
    assert(font_bitmap.texture_height != 0);

    // Calculate normalized coordinates
    float norm_x = static_cast<float>(unsignedNormalizePixelPosition(font_bitmap.current_x_cell * font_bitmap.cell_size, font_bitmap.texture_width));
    float norm_y = static_cast<float>(unsignedNormalizePixelPosition(font_bitmap.current_y_cell * font_bitmap.cell_size, font_bitmap.texture_height));

    FT_Long face_index = FT_Get_Char_Index(face, c); // face->face_index;

    int16_t relative_baseline = ((face->glyph->metrics.height >> 6) - face->glyph->bitmap_top) * 0.85;
    int16_t horizontal_advance = face->glyph->advance.x;

    font_bitmap.char_data.insert({ c,   {   { bitmap_width, bitmap_height, reinterpret_cast<uint8_t *>(current_pixel), relative_baseline, horizontal_advance, face_index },  // CharBitmap
                                            { norm_x, norm_y } }});                                                             // NormalizedPosition

    font_bitmap.current_x_cell++;

    return true;
}

bool setupBitmapForCharacterSet( FontBitmap& font_bitmap,
                                uint16_t texture_width_cells,
                                uint16_t cell_size,
                                uint16_t num_charactors)
{
    font_bitmap.current_x_cell = 0;
    font_bitmap.current_y_cell = 0;
    font_bitmap.texture_width = texture_width_cells * cell_size;
    font_bitmap.texture_height = ((num_charactors / texture_width_cells) + 1) * cell_size;
    font_bitmap.cell_size = cell_size;

//    printf("Texture width -> %d\n", font_bitmap.texture_width);
//    printf("Texture height -> %d\n", font_bitmap.texture_height);

    font_bitmap.bitmap_data = nullptr;
    uint32_t allocation_amount = font_bitmap.texture_width * font_bitmap.texture_height * 4;
    font_bitmap.bitmap_data = (RGBA_8UNORM *) malloc( allocation_amount );

    font_bitmap.char_data.reserve(num_charactors);

    memset(font_bitmap.bitmap_data, 0, allocation_amount);

//    printf("Required space: %u bytes\n", allocation_amount);
//    fflush(stdout);

    return true;
}

void createRectMesh(uint16_t * indices, glm::vec2 * vertices, uint16_t vertices_stride_bytes, uint16_t start_vertex_index, float x, float y, float height, float width)
{
    uint8_t * data = reinterpret_cast<uint8_t *>(vertices);

    *reinterpret_cast<glm::vec2 *>( (data + (0 * vertices_stride_bytes)) ) = {x, y};                     // Top left
    *reinterpret_cast<glm::vec2 *>( (data + (1 * vertices_stride_bytes)) ) = {x + width, y};             // Top right
    *reinterpret_cast<glm::vec2 *>( (data + (2 * vertices_stride_bytes)) ) = {x + width, y + height};    // Bottom right
    *reinterpret_cast<glm::vec2 *>( (data + (3 * vertices_stride_bytes)) ) = {x, y + height};            // Bottom left

//    printf("Pushing back triangle -> \n {%f, %f} {%f, %f},\n {%f, %f} {%f, %f}\n", x, y, x, y + height, x + width, y, x + width, y + height);
    fflush(stdout);

    *(indices + 0) = start_vertex_index;
    *(indices + 1) = start_vertex_index + 1;
    *(indices + 2) = start_vertex_index + 2;
    *(indices + 3) = start_vertex_index;
    *(indices + 4) = start_vertex_index + 2;
    *(indices + 5) = start_vertex_index + 3;
}

void printGlyphInformation(FT_GlyphSlot glyph)
{
    printf("Width: %d\n", glyph->bitmap.width);
    printf("Rows: %d\n", glyph->bitmap.rows);
    printf("Left: %d\n", glyph->bitmap_left);
    printf("Top: %d\n", glyph->bitmap_top);
    printf("X: %ld\n", glyph->advance.x);
    printf("Y: %ld\n", glyph->advance.y);
}

void generateTextMeshes(   uint16_t * indices,
                            glm::vec2 * vertices,
                            uint16_t vertices_stride_bytes,
                            uint16_t start_vertex_index,
                            FontBitmap& font_bitmap,
                            glm::vec2 * texture_map_start,
                            uint16_t texture_map_stride,
                            std::string& text,
                            uint16_t start_x,
                            uint16_t start_y)
{
    float x_pos = static_cast<float>(signedNormalizePixelPosition(start_x, vconfig::INITIAL_WINDOW_WIDTH));
    float y_pos = static_cast<float>(signedNormalizePixelPosition(start_y + TEXT_LINE_SPACING, vconfig::INITIAL_WINDOW_HEIGHT));

    float norm_line_spacing = static_cast<float>(unsignedNormalizePixelPosition(TEXT_LINE_SPACING, vconfig::INITIAL_WINDOW_HEIGHT));

    // float start_x_pos = x_pos;
    double max_width_point = static_cast<double>(x_pos) + unsignedNormalizePixelPosition(MAX_LINE_WIDTH, vconfig::INITIAL_WINDOW_WIDTH);

    float face_width;
    float face_height;

    uint16_t current_text_index = 0;

    for(char c : text)
    {
//        printf("Generating mesh and texture for '%c'\n", c);

        CharBitmap char_data = std::get<0>(font_bitmap.char_data[c]);

        face_width =  static_cast<float>(unsignedNormalizePixelPosition(char_data.width, vconfig::INITIAL_WINDOW_WIDTH));
        face_height = static_cast<float>(unsignedNormalizePixelPosition(char_data.height, vconfig::INITIAL_WINDOW_HEIGHT));

        float norm_relative_baseline = unsignedNormalizePixelPosition(std::get<0>(font_bitmap.char_data[c]).relative_baseline, vconfig::INITIAL_WINDOW_WIDTH);

        /*
         * FT_Get_Kerning( FT_Face     face,
                          FT_UInt     left_glyph,
                          FT_UInt     right_glyph,
                          FT_UInt     kern_mode,
                          FT_Vector  *akerning );
         */

//        printf("H Advance for '%c' -> %d\n", c, char_data.horizontal_advance);

        int16_t relative_kearning_offset = char_data.horizontal_advance / 70;

//        if(text.size() > current_text_index)
//        {
//            puts("Getting kearning!");
//            FT_Vector out_kearning;

//            FT_Long left_index = char_data.glyph_index;
//            FT_Long right_index = std::get<0>(font_bitmap.char_data[text[current_text_index+1]]).glyph_index;

////            assert(left_index != right_index);

//            if(left_index == 0 || right_index == 0)
//                break;

//            puts("Access successful");

//            FT_Get_Kerning(font_bitmap.face, left_index, right_index, FT_KERNING_DEFAULT, &out_kearning);

//            puts("Out of function");
//            relative_kearning_offset = out_kearning.x;

//            printf("Kearning between '%c' and '%c' -> %d\n", c, text[current_text_index + 1], relative_kearning_offset);
//        }

        createRectMesh(indices, vertices, vertices_stride_bytes, start_vertex_index, x_pos, y_pos + (norm_relative_baseline * 1.8) + (norm_line_spacing - face_height), face_height, face_width);

        start_vertex_index += 4;
        indices += 6;

        vertices = reinterpret_cast<glm::vec2 *>( reinterpret_cast<uint8_t *>(vertices) + (4 * vertices_stride_bytes) );
        mapCharTextureToMesh(font_bitmap, c, texture_map_start, texture_map_stride);

        uint8_t * byte_pos = reinterpret_cast<uint8_t *>(texture_map_start);

        // Move texture mapping array pointer forward
        byte_pos += texture_map_stride * 4;
        texture_map_start = reinterpret_cast<glm::vec2 *>(byte_pos);

        // TODO: You're gonna need the advance, kearning, etc. Monospace atm
        x_pos += static_cast<float>(unsignedNormalizePixelPosition(TEXT_SPACING + relative_kearning_offset, vconfig::INITIAL_WINDOW_WIDTH));

        if(x_pos > max_width_point)
        {
            x_pos = max_width_point - static_cast<float>(unsignedNormalizePixelPosition(MAX_LINE_WIDTH, vconfig::INITIAL_WINDOW_WIDTH));
            y_pos += norm_line_spacing;
        }

        current_text_index++;
    }

}
