# fapgauntlet
Hi, I'm anonsunite! This is a fully new developed version of the popular game fapgauntlet.

>What is Fap Gauntlet? It is a fun little fap game you play by following simple instructions in each post in the thread until you reach the end or cum. To contribute post nice images or gifs in the format of (strokes), (speed), (special) where strokes is the total number of strokes written as whole numbers not greater than 199, speed is the strokes per second which may be written as these words (very slow, slow, medium, normal, fast, very fast, extremely fast), and special is whatever pressure or special details or instructions for stroking may be.

Some examples:
>  - 29, slow, kill yourself
>  - 50, fast, eat doritos
>  - 99, very fast, 1 finger
>  - 20, very slow, very soft

> The reason for posting these instructions is because it is more fun to follow them than just an ordinary image dump.

So much stays the same, however, with the new FapGauntlet you can:
   - regulate overall speed
   - read webms, gifs or normal image files
   - insert a short pause between each file
   - "stream" files (it downloads the shit and deletes it when you are done)
   - customize specials for local files
   - reload images if you open it the next time (or not, however you prefer)
   - search in almost all nsfw boards for specific keywords (i.e. "Fap Gauntlet")
   - small browsing 'history' (no duplicates) in the downloadmanager's combobox

So here is the link to the github repository: (eep revisiting, I'll update this shit!)
[github.com/Anonsunite/fapgauntlet](https://github.com/Anonsunite/fapgauntlet)

Sadly, installation will surely be a bit more of a hell than for the old version (which was fucking buggy on my machine!!) since I didn't use Python but C++. You'll need the VLC library and Qt with webengine and webenginewidgets installed. Since I didn't use Windows for a long time, I hope some based anon(s) will help me making installation process on Windows or any other machine less painfull.
Anyways, open a terminal, dear GNU/Linux user.

1. Clone the github repository using `git clone https://github.com/Anonsunite/fapgauntlet`
2. Move into the cloned folder
3. Run `qmake -o Makefile FapGauntlet.pro`
4. Run `make`
5. Enjoy your `FapGauntlet`-executable!

----
#### Some minor issues:

The post format must be exactly `(count), (speed), (special)` without any missing comma or whitespace. It may appear anywhere, but well, still pretty gayfaggotretarded. I'll make this more loose in futre versions. The codebase itself *just* (commit: 51a3568) right now is not that clean as well, this has somewhat high priority for me, as it is more or less braindead work to clean this up. Some minor stability issues were seen, but not often. Most unstable thing are those webms, as libvlc is kinda retarded. And although I'll keep on working on this for some time, I'll need a bit help from you guys. For example, in case you found an issue, please report it so I can fix it. (best would be by using github's issue tracker) yaddyaddyadda, there is more work to do.

----
And now enjoy some fucking fapping, guys.
