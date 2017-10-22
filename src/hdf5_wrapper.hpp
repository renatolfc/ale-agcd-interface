#ifndef AGCD_HDF_WRAPPER
#define AGCD_HDF_WRAPPER

#include <map>
#include <string>
#include <vector>
#include <utility>
#include <iterator>
#include <algorithm>
#include <stdexcept>

#include <hdf5.h>

static const int WIDTH = 160;
static const int HEIGHT = 210;

struct agcd_trajectory_t {
    int frame;
    int reward;
    int score;
    int terminal;
    int action;
};

typedef unsigned char pixel_t;
typedef std::vector<pixel_t> screen_t;

typedef std::pair<std::vector<screen_t>, std::vector<agcd_trajectory_t>> trajectory_t;

typedef std::pair<std::string, size_t> game_pair_t;
/* This is a vector of pairs of trajectory names and sizes */
typedef std::vector<game_pair_t> game_vector_pair_t;
/* This is what we keep in memory */
typedef std::map<std::string, game_vector_pair_t> game_trajectory_t;

template <typename T>
static inline std::vector<T> read_dataset(hid_t loc_id, const char *name, hid_t h5datatype) {
    herr_t status;

    hid_t dataset_id = H5Dopen(loc_id, name, H5P_DEFAULT);
    hid_t copy = dataset_id;

    if (dataset_id < 0) {
        printf("Something bad happened while reading %s.\n", name);
        return std::vector<T>();
    }

    hid_t space_id = H5Dget_space(dataset_id);

    hsize_t n_entries = H5Sget_simple_extent_npoints(space_id);
    H5Sclose(space_id);

    if (n_entries <= 0) {
        H5Dclose(dataset_id);
        return std::vector<T>();
    }

    size_t multiplier;
    if (h5datatype == H5T_NATIVE_UCHAR) {
        multiplier = sizeof(unsigned char);
    } else {
        multiplier = sizeof(int);
    }

    void *data = (void *) malloc(n_entries * multiplier);
    status = H5Dread(dataset_id, h5datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
    T *cast_data = static_cast<T *>(data);
    n_entries = (n_entries * multiplier) / sizeof(T);
    std::vector<T> ret(cast_data, cast_data + n_entries);
    free(data);

    H5Dclose(dataset_id);

    return ret;
}

static herr_t trajectory_info_callback(hid_t loc_id, const char *name, const H5L_info_t *info, void *opdata) {
    H5G_stat_t statbuf;

    H5Gget_objinfo(loc_id, name, false, &statbuf);
    game_vector_pair_t *v = (game_vector_pair_t *)opdata;

    if (statbuf.type != H5G_DATASET) {
        return 0;
    }

    hid_t dataset_id = H5Dopen(loc_id, name, H5P_DEFAULT);
    hid_t space_id = H5Dget_space(dataset_id);

    size_t rank = H5Sget_simple_extent_ndims(space_id);
    hsize_t dims[rank];
    herr_t status = H5Sget_simple_extent_dims(space_id, dims, NULL);

    if (dims[0] == 0) {
        fprintf(stderr, "No elements in dataset %s. "
                "Skipping...\n", name);
        H5Dclose(dataset_id);
        H5Sclose(space_id);
    }

    game_pair_t entry(name, dims[0]);
    v->push_back(entry);

    H5Dclose(dataset_id);
    H5Sclose(space_id);

    return 0;
}

static herr_t file_info_callback(hid_t loc_id, const char *name, const H5L_info_t *info, void *opdata) {
    hsize_t i = 0;
    H5G_stat_t statbuf;

    game_trajectory_t *game_trajectories = (game_trajectory_t *)opdata;

    herr_t ret = H5Gget_objinfo(loc_id, name, false, &statbuf);

    if (statbuf.type == H5G_GROUP) {
        /* Games are top-level groups */
        std::string sname = name;
        std::string screen_path = "/" + sname + "/screens";
        std::string trajectory_path = "/" + sname + "/trajectories";

        hid_t sid = H5Gopen(loc_id, screen_path.c_str(), H5P_DEFAULT);
        hid_t tid = H5Gopen(loc_id, trajectory_path.c_str(), H5P_DEFAULT);

        hsize_t screens_size, trajectories_size;

        if (H5Gget_num_objs(tid, &trajectories_size) < 0) {
            fprintf(stderr, "Failed to get trajectory size for game %s. "
                    "Skipping...\n", name);
            goto cleanup_file_info;
        }

        if (H5Gget_num_objs(sid, &screens_size) < 0) {
            fprintf(stderr, "Failed to get screen size for game %s. "
                    "Skipping...\n", name);
            goto cleanup_file_info;
        }

        if (trajectories_size != screens_size) {
            fprintf(stderr, "Trajectory and screen sizes differ for game %s. "
                    "Skipping...\n", name);
            goto cleanup_file_info;
        }

        H5Literate_by_name(
            loc_id, trajectory_path.c_str(), H5_INDEX_NAME, H5_ITER_NATIVE,
            &i, trajectory_info_callback, &((*game_trajectories)[name]),
            H5P_DEFAULT
        );

cleanup_file_info:
        H5Gclose(sid);
        H5Gclose(tid);
    }
    return 0;
}

class H5Wrapper {
private:
    H5Wrapper();
    hid_t file_id;
    const char *hdf_file;
    std::string current_game;
    bool updating_first = true;
    game_trajectory_t game_trajectories;

public:
    H5Wrapper(const char *hdf_file) : hdf_file(hdf_file) {
        file_id = H5Fopen(hdf_file, H5F_ACC_RDONLY, H5P_DEFAULT);
        if (file_id < 0) {
            throw std::invalid_argument("Unable to open file");
        }
        hsize_t i = 0;
        H5Literate_by_name(
            file_id, "/", H5_INDEX_NAME, H5_ITER_NATIVE, &i,
            file_info_callback, &game_trajectories, H5P_DEFAULT
        );
    }

    ~H5Wrapper() {
        H5Fclose(file_id);
    }

    std::vector<std::string> get_games() {
        std::vector<std::string> ret;

        for (game_trajectory_t::iterator it = game_trajectories.begin(); it != game_trajectories.end(); ++it) {
            ret.push_back(it->first);
        }

        return ret;
    }

    game_vector_pair_t get_trajectories(std::string game) {
        game_trajectory_t::iterator it = game_trajectories.find(game);
        if (it == game_trajectories.end()) {
            return game_vector_pair_t();
        }
        std::sort(it->second.begin(), it->second.end(),
            [](const game_pair_t &a, const game_pair_t &b) -> bool
            {
                return atoi(a.first.c_str()) < atoi(b.first.c_str());
            }
        );
        return it->second;
    }

    trajectory_t get_trajectory(std::string game, std::string trajectory_id) {
        std::vector<agcd_trajectory_t> trajectories;
        trajectories = read_dataset<agcd_trajectory_t>(
            file_id, ("/" + game + "/trajectories/" + trajectory_id).c_str(), H5T_NATIVE_INT
        );

        std::vector<pixel_t> pixels;
        pixels = read_dataset<pixel_t>(
            file_id, ("/" + game + "/screens/" + trajectory_id).c_str(), H5T_NATIVE_UCHAR
        );

        std::vector<screen_t> screens;
        size_t offset = 210 * 160;
        for (register size_t i = 0; i < pixels.size(); i += offset) {
            pixel_t *p = &pixels[i];
            std::vector<pixel_t> tmp(p, p + offset);
            screens.push_back(tmp);
        }

        printf("Trajectories size: %d - Screens size: %d\n", trajectories.size(), screens.size());

        return trajectory_t(screens, trajectories);
    }
};

#endif
