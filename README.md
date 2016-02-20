# Torcs AI Driver
<img width="100% "src="http://i.imgur.com/FQP8CBz.png"/>

[Use with Torcs 1.3.4 only.](https://sourceforge.net/projects/torcs/files/torcs-win32-bin/1.3.4/)
  
Requires the patch in the docs folder to be extracted over the Torcs root directory.

### Line follower driver
- [X] Controls speed based on what's in front of it
- [X] Always follows middle of the track

### Furthest ray driver
- [X] Controls speed similar to line follower
- [X] Always drives towards furthest ray
- [X] Ignores rays blocked by players
- [ ] Actively avoids nearby players
- [ ] Implements hysteresis

### Bezier driver
- [ ] Can be given control points or records a set of control points
- [ ] Based on furthest ray or line follower to go round once if recording control points
- [ ] Follows the calculated bezier curve from points
- [ ] Calculates ideal speed at a given point

### Evolved bezier driver
- [ ] Stores data for each track offline
- [ ] Evolves control points using an ANN to create optimal line for speed

### Realtime driver
- [ ] Doesn't store offline data
- [ ] Trained using evolved bezier
- [ ] Trained to avoid and win against other players