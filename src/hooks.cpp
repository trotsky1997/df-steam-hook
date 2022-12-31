#include "hooks.h"
#include "cache.hpp"
#include "dictionary.h"
#include "screen_manager.h"
#include "ttf_manager.h"
#include "utils.hpp"

void* g_textures_ptr = nullptr;
// cache for textures id
LRUCache<std::string, long> texture_id_cache(500);

SETUP_ORIG_FUNC(add_texture, 0xE827F0);
long __fastcall HOOK(add_texture)(void* ptr, SDL_Surface* texture)
{
  g_textures_ptr = ptr;
  auto ret = ORIGINAL(add_texture)(ptr, texture);
  // spdlog::debug("add texture id {}", ret);
  return ret;
}

// string can be catched here
SETUP_ORIG_FUNC(addst, 0x784C60);
void __fastcall HOOK(addst)(graphicst_* gps, DFString_* str, justification_ justify, int space)
{
  std::string text;
  if (str->len > 15) {
    text = std::string(str->ptr);
  } else {
    text = std::string(str->buf);
  }

  // translation test segment

  // auto translation = Dictionary::GetSingleton()->Get(text);
  // if (translation) {
  //   auto cached = cache->Get(text);
  //   if (cached) {
  //     spdlog::debug("cache search {}", cached.value().get());
  //   }
  //   cache->Put(text, translation.value());

  //   // leak?
  //   DFString_ translated_str{};

  //   // just path does'n work
  //   // auto translated_len = translation.value().size();
  //   // str->len = translated_len;

  //   // if (translated_len > 15) {
  //   //   std::vector<char> cstr(translation.value().c_str(), translation.value().c_str() +
  //   translation.value().size()

  //   //   1); str->ptr = cstr.data(); str->capa = translated_len;
  //   // } else {
  //   //   str->pad = 0;
  //   //   str->capa = 15;
  //   //   strcpy(str->buf, translation.value().c_str());
  //   // }

  //   CreateDFString(translated_str, translation.value());

  //   ORIGINAL(addst)(gps, &translated_str, justify, space);
  //   return;
  // }

  // ttf inject segment
  if (ScreenManager::GetSingleton()->isInitialized() && g_textures_ptr != nullptr) {
    for (int i = 0; i < text.size(); i++) {
      auto texture = TTFManager::GetSingleton()->CreateTexture(text.substr(i, 1));
      auto cached_texture_id = texture_id_cache.Get(text.substr(i, 1));
      long tex_pos;
      if (cached_texture_id) {
        tex_pos = cached_texture_id.value().get();
        // spdlog::debug("texture id from cache {}", cached_texture_id.value().get());
      } else {
        tex_pos = ORIGINAL(add_texture)(g_textures_ptr, texture);
        if (TTFManager::GetSingleton()->cached_response) {
          texture_id_cache.Put(text.substr(i, 1), tex_pos);
        }
        spdlog::debug("new texture id {}", tex_pos);
      }
      auto s = ScreenManager::GetSingleton()->GetTile(gps->screenx + i, gps->screeny);
      s->tex_pos = tex_pos;
    }
  }

  ORIGINAL(addst)(gps, str, justify, space);
  return;
}

// we dont need it at all?
SETUP_ORIG_FUNC(addchar, 0x55D80);
void __fastcall HOOK(addchar)(graphicst_* gps, unsigned char a2, char a3)
{
  ORIGINAL(addchar)(gps, a2, a3);
}

// screen size can be catched here
SETUP_ORIG_FUNC(create_screen, 0x5BB540);
bool __fastcall HOOK(create_screen)(__int64 a1, unsigned int screen_width, unsigned int screen_height)
{
  spdlog::debug("create screen width {}, height {}, ptr 0x{:x}", screen_width, screen_height, (uintptr_t)a1);
  return ORIGINAL(create_screen)(a1, screen_width, screen_height);
}

// resizing font
// can be used to set current font size settings in ttfmanager
SETUP_ORIG_FUNC(reshape, 0x5C0930);
void __fastcall HOOK(reshape)(renderer_2d_base_* renderer, std::pair<int, int> max_grid)
{
  ORIGINAL(reshape)(renderer, max_grid);

  spdlog::debug("reshape dimx {} dimy {} dispx {} dispy {} dispx_z {} dispy_z {} screen 0x{:x}", renderer->dimx,
                renderer->dimy, renderer->dispx, renderer->dispy, renderer->dispx_z, renderer->dispy_z,
                (uintptr_t)renderer->screen);
}

// allocate screen array
SETUP_ORIG_FUNC(gps_allocate, 0x5C2AB0);
void __fastcall HOOK(gps_allocate)(void* ptr, int dimx, int dimy, int screen_width, int screen_height, int dispx_z,
                                   int dispy_z)
{

  spdlog::debug("gps allocate: dimx {} dimy {} screen_width {} screen_height {} dispx_z {} dispy_z {}", dimx, dimy,
                screen_width, screen_height, dispx_z, dispy_z);
  ORIGINAL(gps_allocate)(ptr, dimx, dimy, screen_width, screen_height, dispx_z, dispy_z);
  ScreenManager::GetSingleton()->AllocateScreen(dimx, dimy);
}

// clean screen array here
SETUP_ORIG_FUNC(cleanup_arrays, 0x5C28D0);
void __fastcall HOOK(cleanup_arrays)(void* ptr)
{
  ScreenManager::GetSingleton()->ClearScreen();
  ORIGINAL(cleanup_arrays)(ptr);
}

SETUP_ORIG_FUNC(screen_to_texid, 0x5BAB40);
Either<texture_fullid, texture_ttfid>* __fastcall HOOK(screen_to_texid)(renderer_* renderer, __int64 a2, int x, int y)
{
  Either<texture_fullid, texture_ttfid>* ret = ORIGINAL(screen_to_texid)(renderer, a2, x, y);
  if (ScreenManager::GetSingleton()->isInitialized()) {
    auto s = ScreenManager::GetSingleton()->GetTile(x, y);
    if (s->tex_pos > 0) {
      ret->left.texpos = s->tex_pos;
    }
  }
  return ret;
}

// loading main menu (start game)
SETUP_ORIG_FUNC(load_multi_pdim, 0xE82890);
void __fastcall HOOK(load_multi_pdim)(void* ptr, DFString_* filename, long* tex_pos, long dimx, long dimy,
                                      bool convert_magenta, long* disp_x, long* disp_y)
{
  spdlog::debug("load_multi_pdim: filename {} text_pos {} dimx {} dimy {} convert_magenta {}, disp_x {} disp_y {}",
                filename->ptr, *tex_pos, dimx, dimy, convert_magenta, *disp_x, *disp_y);
  ORIGINAL(load_multi_pdim)(ptr, filename, tex_pos, dimx, dimy, convert_magenta, disp_x, disp_y);
}

// loading mods
SETUP_ORIG_FUNC(load_multi_pdim_2, 0xE82AD0);
void __fastcall HOOK(load_multi_pdim_2)(void* ptr, DFString_* filename, long* tex_pos, long dimx, long dimy,
                                        bool convert_magenta, long* disp_x, long* disp_y)
{
  spdlog::debug("load_multi_pdim2: filename {} text_pos {} dimx {} dimy {} convert_magenta {}, disp_x {} disp_y {}",
                filename->ptr, *tex_pos, dimx, dimy, convert_magenta, *disp_x, *disp_y);
  // if we turn on cache here, game works... but during main menu stage it leaking
  // TODO: find new game/load game global texture reset
  TTFManager::GetSingleton()->cached_response = true;
  ORIGINAL(load_multi_pdim_2)(ptr, filename, tex_pos, dimx, dimy, convert_magenta, disp_x, disp_y);
}

// game init
SETUP_ORIG_FUNC(main_init, 0x87A3C0);
void __fastcall HOOK(main_init)()
{
  spdlog::debug("main init");
  ORIGINAL(main_init)();
}

void InstallHooks()
{
  // init TTFManager
  // should call init() for TTFInit and SDL function load from dll
  // then should load font for drawing text
  auto ttf = TTFManager::GetSingleton();
  ttf->Init();
  ttf->LoadFont("terminus.ttf", 14);

  ATTACH(add_texture);
  ATTACH(addst);
  ATTACH(addchar);
  ATTACH(create_screen);
  ATTACH(reshape);
  ATTACH(load_multi_pdim);
  ATTACH(load_multi_pdim_2);
  ATTACH(screen_to_texid);
  ATTACH(gps_allocate);
  ATTACH(cleanup_arrays);
  ATTACH(main_init);

  spdlog::info("hooks installed");
}