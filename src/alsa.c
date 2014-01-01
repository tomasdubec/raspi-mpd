#include <alsa/asoundlib.h>
#include <math.h>
#include "logging.h"

long alsa_get_volume(void)
{
    long                  l_vol;
    snd_mixer_t          *handle;
    snd_mixer_selem_id_t *sid;
    const char           *card = "default";
    const char           *selem_name = "PCM";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume(elem, 0, &l_vol);

    snd_mixer_close(handle);

    return l_vol;
}

void alsa_set_volume(long l_volume)
{
    long                  l_min;
    long                  l_max;
    snd_mixer_t          *handle;
    snd_mixer_selem_id_t *sid;
    const char           *card = "default";
    const char           *selem_name = "PCM";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &l_min, &l_max);
    snd_mixer_selem_set_playback_volume_all(elem, ((l_max - l_min) / log2(100)) * log2(l_volume) + l_min);

    snd_mixer_close(handle);
}

