#include "driver.hpp"
#include "net/server.hpp"
#include "util/time.hpp"
#include "wrappers/driver.hpp"

#include <chrono>
#include <thread>

#include <csignal>
#include <video/videoPlayer.hpp>

static bool run{ true };

void signalHandler(int dummy)
{
    if (not run) {
        std::exit(-1);
    }
    run = false;
    printf("Quitting\n");
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("missing args");
        return 0;
    }

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    Wrappers::SSD1322 driver;

    // rgb buffer
    uint8_t glBuffer[HardwareSpecs::SSD1322::Size * 3];

    int frameCount{};

    //AnimationController animation(driver.GetWidth(), driver.GetHeight());

    auto current = std::chrono::steady_clock::now();
    auto prev    = std::chrono::steady_clock::now();

    long long int totalFrameTimes{};

    decltype(current) sectionTimes[4];
    long long int sectionDeltas[3]{ 0, 0, 0 };

    int64_t startTime{ util::timing::Get() };

    float now{};

    WebServer server /*(animation)*/;
    std::thread serverThread([&server]() {
        server.Run();
    });

    VideoPlayer player(argv[1], HardwareSpecs::SSD1322::Width, HardwareSpecs::SSD1322::Height);

    while (run) {
        now = static_cast<float>(static_cast<double>(util::timing::Get() - startTime) / static_cast<double>(util::timing::Frequency()));

        //animation.ProcessRequests();

        sectionTimes[0] = std::chrono::steady_clock::now();

        // section 1: openGL operations
        //glWrapper.PreDraw();
        //animation.Draw(now);
        player.GetFrame(glBuffer, sizeof(glBuffer));

        sectionTimes[1] = std::chrono::steady_clock::now();

        // section 2: output buffer conversion and transfer
        // expected to be constant
        //glWrapper.PostDraw(glBuffer);
        driver.CopyGLBuffer(glBuffer);

        sectionTimes[2] = std::chrono::steady_clock::now();

        // section 3: clear buffer, wipe screen
        // expected to be constant
        driver.Display();
        driver.Clear();

        sectionTimes[3] = std::chrono::steady_clock::now();

        current = std::chrono::steady_clock::now();

        //printf("Frametime: %llims\n", delta.count());
        totalFrameTimes += std::chrono::duration_cast<std::chrono::milliseconds>(current - prev).count();
        for (int i = 0; i < 3; i++) {
            auto timeDifference{ std::chrono::duration_cast<std::chrono::microseconds>(sectionTimes[i + 1] - sectionTimes[i]).count() };
            sectionDeltas[i] += timeDifference;
        }
        frameCount++;
        std::swap(current, prev);
    }

    server.Halt();
    serverThread.join();

    printf("Avarage timing:\n");
    printf("Total time:           %07.3lfms\n", static_cast<double>(totalFrameTimes) / frameCount);
    printf("Section 1 (gl)  :   %09.3lfµs\n", static_cast<double>(sectionDeltas[0]) / frameCount);
    printf("Section 2 (copy):   %09.3lfµs\n", static_cast<double>(sectionDeltas[1]) / frameCount);
    printf("Section 3 (cls) :   %09.3lfµs\n", static_cast<double>(sectionDeltas[2]) / frameCount);
}
