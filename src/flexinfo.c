
#include <dryos.h>
#include <property.h>
#include <menu.h>
#include <bmp.h>
#include <config.h>
#include <consts.h>
#include <lens.h>
#include <version.h>
#include <flexinfo.h>

#define BUF_SIZE 128

/* 
   this is the definition of the info screen elements.
   it can either be made switchable for photo and LV setting or put in an array.
   the config can get loaded from an user-save and -editable ini file.
   -> ToDo: for now there is only 7D photo screen, add others too
            do we put raw X/Y positions here or keep them im consts.h?
 */
info_elem_t info_config[64] =
{
    { .config = { { INFO_TYPE_CONFIG } } },

#if defined(CONFIG_7D)
    /* print ISO range */
    { .string = { { INFO_TYPE_STRING, { ISO_RANGE_POS_X, ISO_RANGE_POS_Y, 2 }}, INFO_STRING_ISO_MINMAX, COLOR_YELLOW, INFO_COL_FIELD, INFO_FONT_MEDIUM } },
    
    /* entry 2, WB strings referenced as anchor */
    { .string = { { INFO_TYPE_STRING, { WBS_POS_X, WBS_POS_Y, 2 }}, INFO_STRING_WBS_BA, COLOR_YELLOW, INFO_COL_FIELD, INFO_FONT_LARGE } },
    { .string = { { INFO_TYPE_STRING, { 0, 0, 2, INFO_ANCHOR_RIGHT | INFO_ANCHOR_TOP, 2 }}, INFO_STRING_WBS_GM, COLOR_YELLOW, INFO_COL_FIELD, INFO_FONT_LARGE } },
    
    /* entry 4, battery_icon referenced as anchor */
    { .battery_icon = { { INFO_TYPE_BATTERY_ICON, { DISPLAY_BATTERY_POS_X, DISPLAY_BATTERY_POS_Y, 2 }}, DISPLAY_BATTERY_LEVEL_2, DISPLAY_BATTERY_LEVEL_1 } },
    { .battery_perf = { { INFO_TYPE_BATTERY_PERF, { -14, 0, 3, INFO_ANCHOR_LEFT | INFO_ANCHOR_TOP, 4 }}, 0, 12, 12 } },
    { .string = { { INFO_TYPE_STRING, { 0, 2, 2, INFO_ANCHOR_HCENTER | INFO_ANCHOR_BOTTOM, 4, INFO_ANCHOR_HCENTER | INFO_ANCHOR_TOP }}, INFO_STRING_BATTERY_PCT, COLOR_YELLOW, INFO_COL_FIELD, INFO_FONT_LARGE } },
    { .string = { { INFO_TYPE_STRING, { 0, 0, 2, INFO_ANCHOR_RIGHT | INFO_ANCHOR_TOP, 4 }}, INFO_STRING_BATTERY_ID, COLOR_YELLOW, INFO_COL_FIELD, INFO_FONT_LARGE } },

    /* entry 8, MLU string */
    { .string = { { INFO_TYPE_STRING, { MLU_STATUS_POS_X, MLU_STATUS_POS_Y, 2 }}, INFO_STRING_MLU, COLOR_YELLOW, INFO_COL_FIELD, INFO_FONT_MEDIUM } },
    
    /* entry 9, kelvin */
    { .string = { { INFO_TYPE_STRING, { WB_KELVIN_POS_X, WB_KELVIN_POS_Y, 2 }}, INFO_STRING_KELVIN, COLOR_YELLOW, INFO_COL_FIELD, INFO_FONT_MEDIUM_SHADOW } },
    
    /* entry 10, pictures */
    { .string = { { INFO_TYPE_STRING, { 550, 402, 2 }}, INFO_STRING_PICTURES_4, COLOR_FG_NONLV, INFO_COL_FIELD, INFO_FONT_CANON } },
#endif

    { .type = INFO_TYPE_END },
};

uint32_t info_get_string(char *buffer, uint32_t maxsize, uint32_t string_type)
{
    strcpy(buffer, "");

    switch(string_type)
    {
        case INFO_STRING_ISO:
        {
            snprintf(buffer, maxsize, "(ISO)");
            break;
        }
        case INFO_STRING_ISO_MIN:
        {
            if (lens_info.raw_iso != 0)
            {
                return 1;
            }
            snprintf(buffer, maxsize, "MIN:%d",raw2iso(auto_iso_range >> 8));
            break;
        }
        case INFO_STRING_ISO_MAX:
        {
            if (lens_info.raw_iso != 0)
            {
                return 1;
            }
            snprintf(buffer, maxsize, "MAX:%d",raw2iso(auto_iso_range & 0xFF));
            break;
        }
        case INFO_STRING_ISO_MINMAX:
        {
            if (lens_info.raw_iso != 0)
            {
                return 1;
            }
            snprintf(buffer, maxsize, "[%d-%d]", MAX((get_htp() ? 200 : 100), raw2iso(auto_iso_range >> 8)), raw2iso(auto_iso_range & 0xFF));
            break;
        }
        case INFO_STRING_KELVIN:
        {
            if (lens_info.wb_mode != WB_KELVIN)
            {
                return 1;
            }
            snprintf(buffer, maxsize, "%5d", lens_info.kelvin);
            break;
        }
        case INFO_STRING_WBS_BA:
        {
            int ba = lens_info.wbs_ba;
            if (ba == 0)
            {
                return 1;
            }
            snprintf(buffer, maxsize, "%s%d", ba > 0 ? "A" : "B", ABS(ba));
            break;
        }
        case INFO_STRING_WBS_GM:
        {
            int gm = lens_info.wbs_gm;
            if (gm == 0)
            {
                return 1;
            }
            snprintf(buffer, maxsize, "%s%d", gm > 0 ? "G" : "M", ABS(gm));
            break;
        }
        case INFO_STRING_DATE_DDMMYYYY:
        {
            struct tm now;
            LoadCalendarFromRTC(&now);
            snprintf(buffer, maxsize, "%2d.%2d.%4d", now.tm_mday,(now.tm_mon+1),(now.tm_year+1900));
            break;
        }
        case INFO_STRING_DATE_YYYYMMDD:
        {
            struct tm now;
            LoadCalendarFromRTC(&now);
            snprintf(buffer, maxsize, "%2d.%2d.%4d", (now.tm_mon+1),now.tm_mday,(now.tm_year+1900));
            break;
        }
        case INFO_STRING_DATE_MM:
        {
            struct tm now;
            LoadCalendarFromRTC(&now);
            snprintf(buffer, maxsize, "%2d", (now.tm_mon+1));
            break;
        }
        case INFO_STRING_DATE_DD:
        {
            struct tm now;
            LoadCalendarFromRTC(&now);
            snprintf(buffer, maxsize, "%2d", now.tm_mday);
            break;
        }
        case INFO_STRING_DATE_YY:
        {
            struct tm now;
            LoadCalendarFromRTC(&now);
            snprintf(buffer, maxsize, "%2d", now.tm_year % 100);
            break;
        }
        case INFO_STRING_DATE_YYYY:
        {
            struct tm now;
            LoadCalendarFromRTC(&now);
            snprintf(buffer, maxsize, "%4d", (now.tm_year+1900));
            break;
        }
        case INFO_STRING_TIME:
        {
            struct tm now;
            LoadCalendarFromRTC(&now);
            snprintf(buffer, maxsize, "%02d:%02d", now.tm_hour, now.tm_min);
            break;
        }
        case INFO_STRING_TIME_HH12:
        {
            struct tm now;
            LoadCalendarFromRTC(&now);
            snprintf(buffer, maxsize, "%02d", now.tm_hour % 13);
            break;
        }
        case INFO_STRING_TIME_HH24:
        {
            struct tm now;
            LoadCalendarFromRTC(&now);
            snprintf(buffer, maxsize, "%02d", now.tm_hour);
            break;
        }
        case INFO_STRING_TIME_MM:
        {
            struct tm now;
            LoadCalendarFromRTC(&now);
            snprintf(buffer, maxsize, "%02d", now.tm_min);
            break;
        }
        case INFO_STRING_TIME_SS:
        {
            struct tm now;
            LoadCalendarFromRTC(&now);
            snprintf(buffer, maxsize, "%02d", now.tm_sec);
            break;
        }
        case INFO_STRING_TIME_AMPM:
        {
            struct tm now;
            LoadCalendarFromRTC(&now);
            snprintf(buffer, maxsize, "%s", (now.tm_hour > 12) ? "PM" : "AM");
            break;
        }
        case INFO_STRING_ARTIST:
        {
            snprintf(buffer, maxsize, "%s", artist_name);
            break;
        }
        case INFO_STRING_COPYRIGHT:
        {
            snprintf(buffer, maxsize, "%s", copyright_info);
            break;
        }
        case INFO_STRING_LENS:
        {
            snprintf(buffer, maxsize, "%s", lens_info.name);
            break;
        }
        case INFO_STRING_BUILD:
        {
            snprintf(buffer, maxsize, "%s", build_version);
            break;
        }
        case INFO_STRING_BATTERY_PCT:
        {
            snprintf(buffer, maxsize, "%d%%%%", GetBatteryLevel());
            break;
        }
        case INFO_STRING_BATTERY_ID:
        {
            if (GetBatteryHist() == 0)
            {
                return 1;
            }
            snprintf(buffer, maxsize, "%d", GetBatteryHist());
            break;
        }
        case INFO_STRING_CARD_LABEL_A:
        case INFO_STRING_CARD_LABEL_B:
        case INFO_STRING_CARD_SPACE_A:
        case INFO_STRING_CARD_SPACE_B:
        case INFO_STRING_CARD_FILES_A:
        case INFO_STRING_CARD_FILES_B:
        case INFO_STRING_CARD_MAKER_A:
        case INFO_STRING_CARD_MAKER_B:
        case INFO_STRING_CARD_MODEL_A:
        case INFO_STRING_CARD_MODEL_B:
            snprintf(buffer, maxsize, "(card info)");
            break;
            
        case INFO_STRING_PICTURES_3:
        {
            snprintf(buffer, maxsize, "[%3d%]", avail_shot);
            break;
        }
        case INFO_STRING_PICTURES_4:
        {
            snprintf(buffer, maxsize, "[%4d%]", avail_shot);
            break;
        }
        case INFO_STRING_PICTURES_5:
        {
            snprintf(buffer, maxsize, "[%5d%]", avail_shot);
            break;
        }
        case INFO_STRING_MLU:
        {
            if (get_mlu() == 0)
            {
                return 1;
            }
            snprintf(buffer, maxsize, "MLU");
            break;
        }
        /* error */
        default:
            return 1;
    }

    return 0;
}

uint32_t info_measure_string(char *string, uint32_t font_type, int32_t *width, int32_t *height)
{
    uint32_t font = 0;
    switch(font_type)
    {
        case INFO_FONT_SMALL:
        case INFO_FONT_SMALL_SHADOW:
            font = FONT_SMALL;
            break;
        case INFO_FONT_MEDIUM:
        case INFO_FONT_MEDIUM_SHADOW:
            font = FONT_MED;
            break;
        case INFO_FONT_LARGE:
        case INFO_FONT_LARGE_SHADOW:
            font = FONT_LARGE;
            break;
        case INFO_FONT_CANON:
            font = 0;
            *width = bfnt_puts(string, 1000, 1000, COLOR_CYAN, 0);
            *height = 50;
            break;
        /* error */
        default:
            return 1;
    }
    
    if(font)
    {
        *width = fontspec_font(font)->width * strlen(string);
        *height = fontspec_font(font)->height;
    }

    return 0;
}

uint32_t info_get_absolute(info_elem_t *config, info_elem_t *element)
{
    /* in case of absolute positioning, this is the absolute pos else it is the offset from the anchor */
    element->hdr.pos.abs_x = element->hdr.pos.x;
    element->hdr.pos.abs_y = element->hdr.pos.y;

    /* if the element is relatively positioned to some other element, we have to look it up */

    /* determine position from referenced element identified by 'anchor' and update pos_x, pos_y (they contain the offset) */
    info_elem_t *anchor = &(config[element->hdr.pos.anchor]);

    switch(element->hdr.pos.anchor_flags & INFO_ANCHOR_H_MASK)
    {
        case INFO_ANCHOR_LEFT:
            element->hdr.pos.abs_x += anchor->hdr.pos.x;
            break;
        case INFO_ANCHOR_HCENTER:
            element->hdr.pos.abs_x += anchor->hdr.pos.x + anchor->hdr.pos.w / 2;
            break;
        case INFO_ANCHOR_RIGHT:
            element->hdr.pos.abs_x += anchor->hdr.pos.x + anchor->hdr.pos.w;
            break;
    }

    switch(element->hdr.pos.anchor_flags & INFO_ANCHOR_V_MASK)
    {
        case INFO_ANCHOR_TOP:
            element->hdr.pos.abs_y += anchor->hdr.pos.y;
            break;
        case INFO_ANCHOR_VCENTER:
            element->hdr.pos.abs_y += anchor->hdr.pos.y + anchor->hdr.pos.h / 2;
            break;
        case INFO_ANCHOR_BOTTOM:
            element->hdr.pos.abs_y += anchor->hdr.pos.y + anchor->hdr.pos.h;
            break;
    }

    switch(element->hdr.pos.anchor_flags_self & INFO_ANCHOR_H_MASK)
    {
        case INFO_ANCHOR_LEFT:
            element->hdr.pos.abs_x += 0;
            break;
        case INFO_ANCHOR_HCENTER:
            element->hdr.pos.abs_x += -element->hdr.pos.w / 2;
            break;
        case INFO_ANCHOR_RIGHT:
            element->hdr.pos.abs_x += -element->hdr.pos.w;
            break;
    }

    switch(element->hdr.pos.anchor_flags_self & INFO_ANCHOR_V_MASK)
    {
        case INFO_ANCHOR_TOP:
            element->hdr.pos.abs_y += 0;
            break;
        case INFO_ANCHOR_VCENTER:
            element->hdr.pos.abs_y += -element->hdr.pos.h / 2;
            break;
        case INFO_ANCHOR_BOTTOM:
            element->hdr.pos.abs_y += -element->hdr.pos.h;
            break;
    }
    
    return 0;
}

    
uint32_t info_print_string(info_elem_t *config, info_elem_string_t *element, uint32_t run_type)
{
    char str[BUF_SIZE];

    if(info_get_string(str, BUF_SIZE, element->string_type))
    {
        element->hdr.pos.shown = 0;
        return 1;
    }
    
    /* get absolute position of this element */
    info_get_absolute(config, (info_elem_t *)element);
    int pos_x = element->hdr.pos.abs_x;
    int pos_y = element->hdr.pos.abs_y;

    /* update the width/height */
    info_measure_string(str, element->font_type, &element->hdr.pos.w, &element->hdr.pos.h);

    /* ToDo: make defineable */
    int col_bg = bmp_getpixel(10,1);
    int col_field = bmp_getpixel(615,375);
    uint32_t fgcolor = element->fgcolor;
    uint32_t bgcolor = element->bgcolor;
    uint32_t fnt;

    /* look up special colors. ToDo: optimize */
    if(bgcolor == INFO_COL_BG)
    {
        bgcolor = col_bg;
    }
    if(bgcolor == INFO_COL_FIELD)
    {
        bgcolor = col_field;
    }
    if(fgcolor == INFO_COL_BG)
    {
        fgcolor = col_bg;
    }
    if(fgcolor == INFO_COL_FIELD)
    {
        fgcolor = col_field;
    }

    /* print string if this was not just a pre-pass run */
    if(run_type == INFO_PRINT)
    {
        switch(element->font_type)
        {
            case INFO_FONT_SMALL:
                fnt = FONT(FONT_SMALL, fgcolor, bgcolor);
                bmp_printf(fnt, pos_x, pos_y, str);
                break;
            case INFO_FONT_MEDIUM:
                fnt = FONT(FONT_MED, fgcolor, bgcolor);
                bmp_printf(fnt, pos_x, pos_y, str);
                break;
            case INFO_FONT_LARGE:
                fnt = FONT(FONT_LARGE, fgcolor, bgcolor);
                bmp_printf(fnt, pos_x, pos_y, str);
                break;
            case INFO_FONT_SMALL_SHADOW:
                fnt = SHADOW_FONT(FONT(FONT_SMALL, fgcolor, bgcolor));
                bmp_printf(fnt, pos_x, pos_y, str);
                break;
            case INFO_FONT_MEDIUM_SHADOW:
                fnt = SHADOW_FONT(FONT(FONT_MED, fgcolor, bgcolor));
                bmp_printf(fnt, pos_x, pos_y, str);
                break;
            case INFO_FONT_LARGE_SHADOW:
                fnt = SHADOW_FONT(FONT(FONT_LARGE, fgcolor, bgcolor));
                bmp_printf(fnt, pos_x, pos_y, str);
                break;
            case INFO_FONT_CANON:
                bfnt_puts(str, pos_x, pos_y, fgcolor, bgcolor);
                break;
            /* error */
            default:
                return 1;
        }
    }

    return 0;
}

uint32_t info_print_fill(info_elem_t *config, info_elem_fill_t *element, uint32_t run_type)
{
    bmp_fill(element->color, element->hdr.pos.x, element->hdr.pos.y, element->hdr.pos.w, element->hdr.pos.h);
    return 0;
}

uint32_t info_print_icon(info_elem_t *config, info_elem_icon_t *element, uint32_t run_type)
{
    return 0;
}

uint32_t info_print_battery_perf(info_elem_t *config, info_elem_battery_perf_t *element, uint32_t run_type)
{
    /* get absolute position of this element */
    info_get_absolute(config, (info_elem_t *)element);

    int pos_x = element->hdr.pos.abs_x;
    int pos_y = element->hdr.pos.abs_y;
    int width = element->width;
    int height = element->height;    

    if(element->horizontal)
    {
        element->hdr.pos.w = 3 * width + 8;
        element->hdr.pos.h = height;
    }
    else
    {
        element->hdr.pos.w = width;
        element->hdr.pos.h = 3 * height + 8;
    }
    
    if(run_type == INFO_PRINT)
    {
        if(element->horizontal)
        {
            bmp_fill((GetBatteryPerformance()<1 ? COLOR_GRAY50 : COLOR_GREEN2),pos_x,pos_y,width,height);
            bmp_fill((GetBatteryPerformance()<2 ? COLOR_GRAY50 : COLOR_GREEN2),pos_x+4+width,pos_y,width,height);
            bmp_fill((GetBatteryPerformance()<3 ? COLOR_GRAY50 : COLOR_GREEN2),pos_x+8+2*width,pos_y,width,height);
        }
        else
        {
            bmp_fill((GetBatteryPerformance()<1 ? COLOR_GRAY50 : COLOR_GREEN2),pos_x,pos_y,width,height);
            bmp_fill((GetBatteryPerformance()<2 ? COLOR_GRAY50 : COLOR_GREEN2),pos_x,pos_y+4+height,width,height);
            bmp_fill((GetBatteryPerformance()<3 ? COLOR_GRAY50 : COLOR_GREEN2),pos_x,pos_y+8+2*height,width,height);
        }
    }
    return 0;
}

uint32_t info_print_battery_icon(info_elem_t *config, info_elem_battery_icon_t *element, uint32_t run_type)
{
    int batlev = GetBatteryLevel();
    int col_field = bmp_getpixel(615,455);
    
    /* get absolute position of this element */
    info_get_absolute(config, (info_elem_t *)element);
    int pos_x = element->hdr.pos.abs_x;
    int pos_y = element->hdr.pos.abs_y;

    element->hdr.pos.w = 96;
    element->hdr.pos.h = 32;
    
    if(run_type == INFO_PRINT)
    {
        uint batcol = 0;
        uint batfil = 0;
        bmp_fill(col_field,pos_x-4,pos_y+14,96,32); // clear the Canon battery icon
        
        if (batlev <= (int)element->pct_red)
        {
            batcol = COLOR_RED;
        }
        else
        {
            batcol = COLOR_WHITE;
        }
        
        bmp_fill(batcol,pos_x+10,pos_y,72,32); // draw the new battery icon
        bmp_fill(batcol,pos_x,pos_y+8,12,16);
        bmp_fill(col_field,pos_x+14,pos_y+4,64,24);
        
        if (batlev <= (int)element->pct_red)
        {
            batcol = COLOR_RED;
        }
        else if (batlev <= (int)element->pct_yellow)
        {
            batcol = COLOR_YELLOW;
        }
        else
        {
            batcol = COLOR_GREEN2;
        }
        
        batfil = batlev*56/100;
        bmp_fill(batcol,pos_x+18+56-batfil,pos_y+8,batfil,16);
    }
    return 0;
}


uint32_t info_get_next_z(info_elem_t *config, uint32_t current)
{
    uint32_t pos = 0;
    uint32_t next = INFO_Z_END;

    while(config[pos].type != INFO_TYPE_END)
    {
        uint32_t z = config[pos].hdr.pos.z;
        
        if(z >= current && z < next )
        {
            next = z;
        }
        pos++;
    }

    return next;
}

uint32_t info_print_element(info_elem_t *config, info_elem_t *element, uint32_t run_type)
{
    switch(element->type)
    {
        case INFO_TYPE_STRING:
            return info_print_string(config, (info_elem_string_t *)element, run_type);
        case INFO_TYPE_BATTERY_ICON:
            return info_print_battery_icon(config, (info_elem_battery_icon_t *)element, run_type);
        case INFO_TYPE_BATTERY_PERF:
            return info_print_battery_perf(config, (info_elem_battery_perf_t *)element, run_type);
        case INFO_TYPE_FILL:
            return info_print_fill(config, (info_elem_fill_t *)element, run_type);
        case INFO_TYPE_ICON:
            return info_print_icon(config, (info_elem_icon_t *)element, run_type);
    }
    
    return 1;
}

uint32_t info_print_config(info_elem_t *config)
{
    uint32_t pos = 0;
    int32_t z = 0;

    while(config[pos].type != INFO_TYPE_END)
    {
        /* by default nothing is shown, so reset all shown flags */
        config[pos].hdr.pos.shown = 0;
        pos++;
    }
    
    pos = 0;
    while(config[pos].type != INFO_TYPE_END)
    { 
        /* but let check if the element should get shown. this updates above flag and ensures that lower layers are only drawn if the referenced is shown */
        info_print_element(config, &(config[pos]), INFO_PRERUN);
        pos++;
    }

    z = info_get_next_z(config, 0);
    while(z != INFO_Z_END)
    {
        pos = 0;
        while(config[pos].type != INFO_TYPE_END)
        {
            if(z == config[pos].hdr.pos.z)
            {
                info_print_element(config, &(config[pos]), INFO_PRINT);
                
                if(config[0].config.show_boundaries || config[0].config.selected_item == pos || config[0].config.anchor_target == pos)
                {
                    int color = COLOR_RED;
                    
                    /* the currently selected item is drawn green and the anchor target is drawn blue */
                    if(config[0].config.selected_item == pos)
                    {
                        color = COLOR_GREEN1;
                    }
                    else if(config[0].config.anchor_target == pos)
                    {
                        color = COLOR_BLUE;
                    }
                    
                    /* very small sized elements will get drawn as blocks */
                    if(config[pos].hdr.pos.w > 4 && config[pos].hdr.pos.h > 4)
                    {
                        draw_line(config[pos].hdr.pos.abs_x, config[pos].hdr.pos.abs_y, config[pos].hdr.pos.abs_x + config[pos].hdr.pos.w, config[pos].hdr.pos.abs_y, color);
                        draw_line(config[pos].hdr.pos.abs_x, config[pos].hdr.pos.abs_y, config[pos].hdr.pos.abs_x, config[pos].hdr.pos.abs_y + config[pos].hdr.pos.h, color);
                        draw_line(config[pos].hdr.pos.abs_x + config[pos].hdr.pos.w, config[pos].hdr.pos.abs_y + config[pos].hdr.pos.h, config[pos].hdr.pos.abs_x, config[pos].hdr.pos.abs_y + config[pos].hdr.pos.h, color);
                        draw_line(config[pos].hdr.pos.abs_x + config[pos].hdr.pos.w, config[pos].hdr.pos.abs_y + config[pos].hdr.pos.h, config[pos].hdr.pos.abs_x + config[pos].hdr.pos.w, config[pos].hdr.pos.abs_y, color);
                        draw_line(config[pos].hdr.pos.abs_x, config[pos].hdr.pos.abs_y, config[pos].hdr.pos.abs_x + config[pos].hdr.pos.w, config[pos].hdr.pos.abs_y + config[pos].hdr.pos.h, color);
                        draw_line(config[pos].hdr.pos.abs_x + config[pos].hdr.pos.w, config[pos].hdr.pos.abs_y, config[pos].hdr.pos.abs_x, config[pos].hdr.pos.abs_y + config[pos].hdr.pos.h, color);
                    }
                    else
                    {
                        bmp_fill(color,config[pos].hdr.pos.abs_x,config[pos].hdr.pos.abs_y,8,8);
                    }
                }
            }
            pos++;
        }
        /* find the next highest layers */
        z = info_get_next_z(config, z + 1);
    } 
    return 0;
}


uint32_t info_print_screen()
{
    return info_print_config(info_config);
}

void info_menu_item_select(void* priv, int delta)
{
    uint32_t count = 0;
    info_elem_t *config = (info_elem_t *)priv;

    while(config[count].type != INFO_TYPE_END)
    {
        count++;
    }
    
    if((delta < 0 && config[0].config.selected_item > 0) || (delta > 0 && config[0].config.selected_item < count))
    {
        config[0].config.selected_item += delta;
    }
}

void info_menu_item_display(void *priv, int x, int y, int selected)
{
    info_elem_t *config = (info_elem_t *)priv;
    
    if(config[0].config.selected_item)
    {
        bmp_printf(selected ? MENU_FONT_SEL : MENU_FONT, x, y, "Select item: #%d", config[0].config.selected_item);
        if(selected)
        {
            info_print_config(config);
        }
    }
    else
    {
        bmp_printf(selected ? MENU_FONT_SEL : MENU_FONT, x, y, "Select item: (none)");
    }
}

void info_menu_item_posx_select(void* priv, int delta)
{
    info_elem_t *config = (info_elem_t *)priv;
    info_elem_t *item = (info_elem_t *) &config[config[0].config.selected_item];
    
    if((delta < 0 && item->hdr.pos.x > -50) || (delta > 0 && item->hdr.pos.x < 720))
    {
        item->hdr.pos.x += delta;
    }
}

void info_menu_item_posy_select(void* priv, int delta)
{
    info_elem_t *config = (info_elem_t *)priv;
    info_elem_t *item = (info_elem_t *) &config[config[0].config.selected_item];
    
    if((delta < 0 && item->hdr.pos.y > -50) || (delta > 0 && item->hdr.pos.y < 480))
    {
        item->hdr.pos.y += delta;
    }
}

void info_menu_item_posz_select(void* priv, int delta)
{
    info_elem_t *config = (info_elem_t *)priv;
    info_elem_t *item = (info_elem_t *) &config[config[0].config.selected_item];
    
    if((delta < 0 && item->hdr.pos.z > 0) || (delta > 0 && item->hdr.pos.z < 32))
    {
        item->hdr.pos.z += delta;
    }
}

void info_menu_item_posx_display(void *priv, int x, int y, int selected)
{
    info_elem_t *config = (info_elem_t *)priv;
    
    if(config[0].config.selected_item)
    {
        bmp_printf(selected ? MENU_FONT_SEL : MENU_FONT, x, y, "X-Position: #%d", config[config[0].config.selected_item].hdr.pos.x);
        if(selected)
        {
            info_print_config(config);
        }
    }
    else
    {
        bmp_printf(selected ? MENU_FONT_SEL : MENU_FONT, x, y, "X-Position: (none)");
    }
}

void info_menu_item_posy_display(void *priv, int x, int y, int selected)
{
    info_elem_t *config = (info_elem_t *)priv;
    
    if(config[0].config.selected_item)
    {
        bmp_printf(selected ? MENU_FONT_SEL : MENU_FONT, x, y, "Y-Position: #%d", config[config[0].config.selected_item].hdr.pos.y);
        if(selected)
        {
            info_print_config(config);
        }
    }
    else
    {
        bmp_printf(selected ? MENU_FONT_SEL : MENU_FONT, x, y, "Y-Position: (none)");
    }
}

void info_menu_item_posz_display(void *priv, int x, int y, int selected)
{
    info_elem_t *config = (info_elem_t *)priv;
    
    if(config[0].config.selected_item)
    {
        bmp_printf(selected ? MENU_FONT_SEL : MENU_FONT, x, y, "Z-Position: #%d", config[config[0].config.selected_item].hdr.pos.z);
        if(selected)
        {
            info_print_config(config);
        }
    }
    else
    {
        bmp_printf(selected ? MENU_FONT_SEL : MENU_FONT, x, y, "Z-Position: (none)");
    }
}




void info_menu_item_anchor_select(void* priv, int delta)
{
    info_elem_t *config = (info_elem_t *)priv;
    info_elem_t *item = (info_elem_t *) &config[config[0].config.selected_item];

    if((delta < 0 && item->hdr.pos.anchor_flags > 0) || (delta > 0 && item->hdr.pos.anchor_flags < 15))
    {
        item->hdr.pos.anchor_flags += delta;
    }
}

void info_menu_item_anchor_display(void *priv, int x, int y, int selected)
{
    info_elem_t *config = (info_elem_t *)priv;
    const char *text[] = {
        "Absolute",
        "Left", "H-Center", "Right",
        "Top",
        "Top-Left", "Top-Center", "Top-Right", 
        "V-Center",
        "Left-Center", "Center", "Right-Center",
        "Bottom",
        "Bottom-Left", "Bottom-Center", "Bottom-Right"};

    if(config[0].config.selected_item)
    {
        bmp_printf(selected ? MENU_FONT_SEL : MENU_FONT, x, y, "Anchored: %s", text[config[config[0].config.selected_item].hdr.pos.anchor_flags]);
        if(selected)
        {
            info_print_config(config);
        }
    }
    else
    {
        bmp_printf(selected ? MENU_FONT_SEL : MENU_FONT, x, y, "Anchored: (none)");
    }
}

void info_menu_item_anchor_item_select(void* priv, int delta)
{
    uint32_t count = 0;
    info_elem_t *config = (info_elem_t *)priv;
    info_elem_t *item = (info_elem_t *) &config[config[0].config.selected_item];

    while(config[count].type != INFO_TYPE_END)
    {
        count++;
    }
    
    if((delta < 0 && item->hdr.pos.anchor > 0) || (delta > 0 && item->hdr.pos.anchor < count))
    {
        item->hdr.pos.anchor += delta;
    }
}

void info_menu_item_anchor_item_display(void *priv, int x, int y, int selected)
{
    info_elem_t *config = (info_elem_t *)priv;
    info_elem_t *item = (info_elem_t *) &config[config[0].config.selected_item];
    
    if(config[0].config.selected_item)
    {
        bmp_printf(selected ? MENU_FONT_SEL : MENU_FONT, x, y, "Anchor item: #%d", item->hdr.pos.anchor);
        if(selected)
        {
            info_print_config(config);
        }
    }
    else
    {
        bmp_printf(selected ? MENU_FONT_SEL : MENU_FONT, x, y, "Anchor item: (none)");
    }
}


void info_menu_item_anchor_self_select(void* priv, int delta)
{
    info_elem_t *config = (info_elem_t *)priv;
    info_elem_t *item = (info_elem_t *) &config[config[0].config.selected_item];

    if((delta < 0 && item->hdr.pos.anchor_flags_self > 0) || (delta > 0 && item->hdr.pos.anchor_flags_self < 15))
    {
        item->hdr.pos.anchor_flags_self += delta;
    }
}

void info_menu_item_anchor_self_display(void *priv, int x, int y, int selected)
{
    info_elem_t *config = (info_elem_t *)priv;
    const char *text[] = {
        "Absolute",
        "Left", "H-Center", "Right",
        "Top",
        "Top-Left", "Top-Center", "Top-Right", 
        "V-Center",
        "Left-Center", "Center", "Right-Center",
        "Bottom",
        "Bottom-Left", "Bottom-Center", "Bottom-Right" };

    if(config[0].config.selected_item)
    {
        bmp_printf(selected ? MENU_FONT_SEL : MENU_FONT, x, y, "Own anchor: %s", text[config[config[0].config.selected_item].hdr.pos.anchor_flags_self]);
        if(selected)
        {
            info_print_config(config);
        }
    }
    else
    {
        bmp_printf(selected ? MENU_FONT_SEL : MENU_FONT, x, y, "Own anchor: (none)");
    }
}

static struct menu_entry info_menus[] = {
    {
        .name = "FlexInfo Settings",
        .select = menu_open_submenu,
        .submenu_width = 700,
        .children =  (struct menu_entry[]) {
            {
                .name = "Show boundaries",
                .priv = &(info_config[0].config.show_boundaries),
                .max = 1,
                .help = "Enable boundary display for all elements.",
            },
            {
                .name = "Select item",
                .priv = info_config,
                .min = 0,
                .max = 64,
                .select = info_menu_item_select,
                .display = info_menu_item_display,
                .help = "Select a specific element for editing.",
            },
            {
                .name = "Pos X",
                .priv = info_config,
                .min = 0,
                .max = 720,
                .select = info_menu_item_posx_select,
                .display = info_menu_item_posx_display,
                .help = "Move item in its X position.",
            },
            {
                .name = "Pos Y",
                .priv = info_config,
                .min = 0,
                .max = 480,
                .select = info_menu_item_posy_select,
                .display = info_menu_item_posy_display,
                .help = "Move item in its Y position.",
            },
            {
                .name = "Pos Z",
                .priv = info_config,
                .min = 0,
                .max = 32,
                .select = info_menu_item_posz_select,
                .display = info_menu_item_posz_display,
                .help = "Move item in its Z position.",
            },
            {
                .name = "Anchor type",
                .priv = info_config,
                .min = 0,
                .max = 9,
                .select = info_menu_item_anchor_select,
                .display = info_menu_item_anchor_display,
                .help = "Select anchor tyoe",
            },
            {
                .name = "Anchor item",
                .priv = info_config,
                .select = info_menu_item_anchor_item_select,
                .display = info_menu_item_anchor_item_display,
                .help = "Select Anchor item.",
            },
            {
                .name = "Anchor on self",
                .priv = info_config,
                .min = 0,
                .max = 9,
                .select = info_menu_item_anchor_self_select,
                .display = info_menu_item_anchor_self_display,
                .help = "Select anchor tyoe",
            },
            MENU_EOL,
        }
    }
};

static void info_init()
{
    menu_add( "Prefs", info_menus, COUNT(info_menus) );
}

static void info_edit_task()
{
    TASK_LOOP
    {
        if (info_config[0].config.selected_item || info_config[0].config.show_boundaries)
        {
            info_print_config(info_config);
            msleep(50);
        }
        else
        {
            msleep(500);
        }
    }
}

TASK_CREATE( "info_edit_task", info_edit_task, 0, 0x16, 0x1000 );
INIT_FUNC("info.init", info_init);
