#include <algorithm>

#include "sprite.h"
#include "csvdata.h"

namespace Ossium
{

    inline namespace animator
    {

        ///
        /// SpriteAnimation
        ///

        REGISTER_ANIMATION(SpriteAnimation);

        bool SpriteAnimation::Load(string path)
        {
            CSV csv;
            if (csv.Import(path, false))
            {
                if (csv.data[0].empty() || csv.data[0].size() < 4)
                {
                    SDL_Log("Error getting animation data from sprite animation file '%s'.", path.c_str());
                    return false;
                }
                name = splitPair(csv.GetCell(0, 0), '=');
                spriteSheetPath = splitPair(csv.GetCell(0, 1), '=');
                loop = splitPair(csv.GetCell(0, 2), '=') == "true" ? true : false;
                duration = (Uint32)ToInt(splitPair(csv.GetCell(0, 3), '='));
                tween = splitPair(csv.GetCell(0, 4), '=') == "true" ? true : false;
                if (spriteSheetPath == "" || spriteSheetPath == "UNKNOWN")
                {
                    SDL_Log("Error getting sprite sheet path from sprite animation file '%s'.", path.c_str());
                    return false;
                }
                if (tween)
                {
                    for (unsigned int i = 2, counti = csv.data.empty() ? 0 : csv.data.size(); i < counti; i++)
                    {
                        SpriteKeyframe keyframe;
                        keyframe.timePosition = (Uint32)ToInt(splitPairFirst(csv.GetCell(i, 0), ':'));
                        keyframe.clipArea.x = (Uint32)ToInt(splitPairFirst(csv.GetCell(i, 1), ':'));
                        keyframe.clipArea.y = (Uint32)ToInt(splitPairFirst(csv.GetCell(i, 2), ':'));
                        keyframe.clipArea.w = (Uint32)ToInt(splitPairFirst(csv.GetCell(i, 3), ':'));
                        keyframe.clipArea.h = (Uint32)ToInt(splitPairFirst(csv.GetCell(i, 4), ':'));
                        keyframe.position.x = ToFloat(splitPairFirst(csv.GetCell(i, 5), ':'));
                        keyframe.position.y = ToFloat(splitPairFirst(csv.GetCell(i, 6), ':'));
                        keyframe.width = ToFloat(splitPairFirst(csv.GetCell(i, 7), ':'));
                        keyframe.height = ToFloat(splitPairFirst(csv.GetCell(i, 8), ':'));
                        keyframe.origin.x = ToFloat(splitPairFirst(csv.GetCell(i, 9), ':'));
                        keyframe.origin.y = ToFloat(splitPairFirst(csv.GetCell(i, 10), ':'));
                        keyframe.angle = ToFloat(splitPairFirst(csv.GetCell(i, 11), ':'));
                        /// Now the tweening transitions
                        keyframe.transitions[0] = (Uint8)ToInt(splitPair(csv.GetCell(i, 1), ':', "0"));
                        keyframe.transitions[1] = (Uint8)ToInt(splitPair(csv.GetCell(i, 2), ':', "0"));
                        keyframe.transitions[2] = (Uint8)ToInt(splitPair(csv.GetCell(i, 3), ':', "0"));
                        keyframe.transitions[3] = (Uint8)ToInt(splitPair(csv.GetCell(i, 4), ':', "0"));
                        keyframe.transitions[4] = (Uint8)ToInt(splitPair(csv.GetCell(i, 5), ':', "0"));
                        keyframe.transitions[5] = (Uint8)ToInt(splitPair(csv.GetCell(i, 6), ':', "0"));
                        keyframe.transitions[6] = (Uint8)ToInt(splitPair(csv.GetCell(i, 7), ':', "0"));
                        keyframe.transitions[7] = (Uint8)ToInt(splitPair(csv.GetCell(i, 8), ':', "0"));
                        keyframe.transitions[8] = (Uint8)ToInt(splitPair(csv.GetCell(i, 9), ':', "0"));
                        keyframe.transitions[9] = (Uint8)ToInt(splitPair(csv.GetCell(i, 10), ':', "0"));
                        keyframe.transitions[10] = (Uint8)ToInt(splitPair(csv.GetCell(i, 11), ':', "0"));
                        /// Insert the loaded keyframe
                        keyframes.insert(keyframe);
                    }
                }
                else
                {
                    for (unsigned int i = 2, counti = csv.data.empty() ? 0 : csv.data.size(); i < counti; i++)
                    {
                        SpriteKeyframe keyframe;
                        keyframe.timePosition = (Uint32)ToInt(csv.GetCell(i, 0));
                        keyframe.clipArea.x = (Uint32)ToInt(csv.GetCell(i, 1));
                        keyframe.clipArea.y = (Uint32)ToInt(csv.GetCell(i, 2));
                        keyframe.clipArea.w = (Uint32)ToInt(csv.GetCell(i, 3));
                        keyframe.clipArea.h = (Uint32)ToInt(csv.GetCell(i, 4));
                        keyframe.position.x = ToFloat(csv.GetCell(i, 5));
                        keyframe.position.y = ToFloat(csv.GetCell(i, 6));
                        keyframe.width = ToFloat(csv.GetCell(i, 7));
                        keyframe.height = ToFloat(csv.GetCell(i, 8));
                        keyframe.origin.x = ToFloat(csv.GetCell(i, 9));
                        keyframe.origin.y = ToFloat(csv.GetCell(i, 10));
                        keyframe.angle = ToFloat(csv.GetCell(i, 11));
                        /// Insert the loaded keyframe
                        keyframes.insert(keyframe);
                    }
                }
                return Image::Load(spriteSheetPath);
            }
            return false;
        }

        bool SpriteAnimation::LoadAndInit(string path, Renderer& renderer, Uint32 windowPixelFormat, bool cache)
        {
            return Load(path) && Image::Init(renderer, windowPixelFormat, cache);
        }

        void SpriteAnimation::Export(string path)
        {
            Export(path, "");
        }

        void SpriteAnimation::Export(string path, string spriteSheet)
        {
            CSV csv;
            spriteSheet = spriteSheet.length() == 0 ? (spriteSheetPath.length() == 0 ? "UNKNOWN" : spriteSheetPath) : spriteSheet;
            csv.data.push_back({
                "name=" + name,
                "spritesheet=" + spriteSheet,
                "loopable=" + (string)(loop ? "true" : "false"),
                "duration=" + ToString((int)duration),
                "tweening=" + (string)(tween ? "true" : "false")
            });
            /// Add the headers.
            csv.data.push_back({
                "Time position (ms):",
                "Clip Area x:",
                "Clip Area y:",
                "Clip Area w:",
                "Clip Area h:",
                "Position x:",
                "Position y:",
                "Render w:",
                "Render h:",
                "Origin x:",
                "Origin y:",
                "Angle:"
            });
            /// Now chuck in the data from all the keyframes in this animation
            if (tween)
            {
                for (SpriteKeyframe keyframe : keyframes)
                {
                    csv.data.push_back({
                        ToString((int)keyframe.timePosition),
                        ToString(keyframe.clipArea.x) + ":" + ToString((int)keyframe.transitions[0]),
                        ToString(keyframe.clipArea.y) + ":" + ToString((int)keyframe.transitions[1]),
                        ToString(keyframe.clipArea.w) + ":" + ToString((int)keyframe.transitions[2]),
                        ToString(keyframe.clipArea.h) + ":" + ToString((int)keyframe.transitions[3]),
                        ToString(keyframe.position.x) + ":" + ToString((int)keyframe.transitions[4]),
                        ToString(keyframe.position.y) + ":" + ToString((int)keyframe.transitions[5]),
                        ToString(keyframe.width) + ":" + ToString((int)keyframe.transitions[6]),
                        ToString(keyframe.height) + ":" + ToString((int)keyframe.transitions[7]),
                        ToString(keyframe.origin.x) + ":" + ToString((int)keyframe.transitions[8]),
                        ToString(keyframe.origin.y) + ":" + ToString((int)keyframe.transitions[9]),
                        ToString(keyframe.angle) + ":" + ToString((int)keyframe.transitions[10])
                    });
                }
            }
            else
            {
                for (SpriteKeyframe keyframe : keyframes)
                {
                    csv.data.push_back({
                        ToString((int)keyframe.timePosition),
                        ToString(keyframe.clipArea.x),
                        ToString(keyframe.clipArea.y),
                        ToString(keyframe.clipArea.w),
                        ToString(keyframe.clipArea.h),
                        ToString(keyframe.position.x),
                        ToString(keyframe.position.y),
                        ToString(keyframe.width),
                        ToString(keyframe.height),
                        ToString(keyframe.origin.x),
                        ToString(keyframe.origin.y),
                        ToString(keyframe.angle)
                    });
                }
            }
            csv.Export(path, false);
        }

        SpriteKeyframe SpriteAnimation::GetSample(AnimatorClip* clip)
        {
            AnimatorTimeline* tl = clip->GetTimeline();
            if (!keyframes.empty() && tl != nullptr)
            {
                /// Find the closest keyframe with lower_bound
                SpriteKeyframe interpolated;
                interpolated.timePosition = clip->GetTime();
                auto sample = --lower_bound(keyframes.begin(), keyframes.end(), interpolated);
                if (sample == keyframes.end())
                {
                    /// Animation sample time is less than the first keyframe time, so return the first keyframe.
                    return *keyframes.begin();
                }
                interpolated = *sample;

                if (tween)
                {
                    /// Attempt to interpolate between the sample keyframe and the next keyframe
                    sample++;
                    float percent;
                    if (sample == keyframes.end())
                    {
                        sample = keyframes.begin();
                        percent = (float)(clip->GetTime() - interpolated.timePosition) / (float)((duration - interpolated.timePosition) + sample->timePosition);
                    }
                    else
                    {
                        percent = (float)(clip->GetTime() - interpolated.timePosition) / (float)(sample->timePosition - interpolated.timePosition);
                    }

                    /// Attempt to interpolate each member
                    interpolated.clipArea.x = AttemptTween((float)interpolated.clipArea.x, (float)sample->clipArea.x, percent, tl->GetTweeningFunc(interpolated.transitions[0]));
                    interpolated.clipArea.y = AttemptTween((float)interpolated.clipArea.y, (float)sample->clipArea.y, percent, tl->GetTweeningFunc(interpolated.transitions[1]));
                    interpolated.clipArea.w = AttemptTween((float)interpolated.clipArea.w, (float)sample->clipArea.w, percent, tl->GetTweeningFunc(interpolated.transitions[2]));
                    interpolated.clipArea.h = AttemptTween((float)interpolated.clipArea.h, (float)sample->clipArea.h, percent, tl->GetTweeningFunc(interpolated.transitions[3]));
                    interpolated.position.x = AttemptTween(interpolated.position.x, sample->position.x, percent, tl->GetTweeningFunc(interpolated.transitions[4]));
                    interpolated.position.y = AttemptTween(interpolated.position.y, sample->position.y, percent, tl->GetTweeningFunc(interpolated.transitions[5]));
                    interpolated.width = AttemptTween(interpolated.width, sample->width, percent, tl->GetTweeningFunc(interpolated.transitions[6]));
                    interpolated.height = AttemptTween(interpolated.height, sample->height, percent, tl->GetTweeningFunc(interpolated.transitions[7]));
                    interpolated.origin.x = AttemptTween(interpolated.origin.x, sample->origin.x, percent, tl->GetTweeningFunc(interpolated.transitions[8]));
                    interpolated.origin.y = AttemptTween(interpolated.origin.y, sample->origin.y, percent, tl->GetTweeningFunc(interpolated.transitions[9]));
                    interpolated.angle = AttemptTween(interpolated.angle, sample->angle, percent, tl->GetTweeningFunc(interpolated.transitions[10]));
                }

                return interpolated;
            }
            return defaultKeyframe;
        }

    }

    ///
    /// Sprite
    ///

    REGISTER_COMPONENT(Sprite);

    void Sprite::PlayAnimation(AnimatorTimeline& timeline, SpriteAnimation* animation, int startTime, int loops, bool autoRemove)
    {
        animation->SetDefaultKeyframeToFirstKeyframe();
        anim.SetAnimation(animation);
        SetSource(static_cast<Image*>(animation));
        anim.SetLoops(loops);
        anim.Play(timeline, startTime, autoRemove);
    }

    void Sprite::Update()
    {
        SpriteKeyframe kf = anim.Sample<SpriteKeyframe>();
        SetClip(kf.clipArea.x, kf.clipArea.y, kf.clipArea.w, kf.clipArea.h);
        position -= offset;
        offset = kf.position;
        position += offset;
        width = kf.width;
        height = kf.height;
        angle = kf.angle;
    }

}