#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <wayland-client.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "xdg-shell.h"

static xdg_wm_base *xdg_shell = nullptr;
static wl_shm *shm = nullptr;
static wl_compositor *compositor = nullptr;

static void handle_configure(void *data, xdg_surface *surface, uint32_t serial) {
    xdg_surface_ack_configure(surface, serial);
}

static const xdg_surface_listener surface_listener = {
    .configure = handle_configure
};

static void handle_global(void *data, wl_registry *registry,
                          uint32_t name, const char *interface, uint32_t version) {
    if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        xdg_shell = (xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);

    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        shm = (wl_shm*)wl_registry_bind(registry, name, &wl_shm_interface, 1);

    } else if (strcmp(interface, wl_compositor_interface.name) == 0) {
        compositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 1);
    }
}

static const wl_registry_listener registry_listener = {
    .global = handle_global
};

static wl_buffer *create_buffer() {
    const int width = 100;
    const int height = 100;
    const int bytes_per_pixel = 4;
    const int size = width * height * bytes_per_pixel;
    const int stride = width * bytes_per_pixel;

    int fd = shm_open("/buffer", O_RDWR | O_CREAT, DEFFILEMODE);
    ftruncate(fd, size);

    void *shm_data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
    wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);

    memset(shm_data, 255, size);
    munmap(shm_data, size);

    return buffer;
}

int main(int argc, char** argv) {
    wl_display *display = wl_display_connect(nullptr);
    if (display == nullptr) {
        fprintf(stderr, "failed to connect to display\n");
        return EXIT_FAILURE;
    }

    wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, nullptr);

    wl_display_dispatch(display);
    wl_display_roundtrip(display);

    wl_buffer *buffer = create_buffer();

    wl_surface *surface = wl_compositor_create_surface(compositor);

    xdg_surface *shell_surface = xdg_wm_base_get_xdg_surface(xdg_shell, surface);
    xdg_toplevel *toplevel = xdg_surface_get_toplevel(shell_surface);
    (void)toplevel;

    xdg_surface_add_listener(shell_surface, &surface_listener, nullptr);

    wl_surface_commit(surface);
    wl_display_roundtrip(display);

    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_commit(surface);

    while (wl_display_dispatch(display) != -1) {
        // handle events
    }

    return EXIT_SUCCESS;
}
