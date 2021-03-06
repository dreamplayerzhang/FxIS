#ifndef IMAGESTREAMREQUESTER_H
#define IMAGESTREAMREQUESTER_H

#include <vector>
#include <thread>
#include <memory>
#include <functional>
#include <future>
#include <opencv2/opencv.hpp>
#include "ImageStream.h"
#include "ThreadsafeQueue.h"
#include "EventObject.h"
#include "TimeMeasure.h"
#include "ExtendedImageStream.h"

struct AsyncImageRequest {
    TimePoint timestamp;
    std::shared_ptr<std::promise<bool>> promise_ptr;
};

void imageRequestThread(
        ImageStream& im_stream,
        ImageResponse& resp,
        ThreadsafeQueue<AsyncImageRequest>& q,
        EventObject& stop_event
);

template <class ResT>
void extendedImageRequestThread(
        ExtendedImageStream<ResT>& im_stream,
        ImageResponse& resp,
        ResT& processing_result,
        ThreadsafeQueue<AsyncImageRequest>& q,
        EventObject& stop_event
);


class BaseImageStreamRequester {

public:

    virtual void start() = 0;

    std::future<bool> requestImage(TimePoint t) {

        std::shared_ptr<std::promise<bool>> promise_ptr(new std::promise<bool>{});
        AsyncImageRequest req{
                t,
                promise_ptr
        };

        std::future<bool> future = req.promise_ptr->get_future();

        this->q_in.push(req);

        return future;

    };

protected:

    ThreadsafeQueue<AsyncImageRequest> q_in;
    EventObject eo_stop;
    std::unique_ptr<std::thread> thread_ptr;

};

class ImageStreamRequester : public BaseImageStreamRequester {

public:

    explicit ImageStreamRequester(ImageStream& im_stream);

    void start() override;

    virtual void copyData(ImageResponse& out);

private:

    ImageStream& image_stream;
    ImageResponse image_response;

};

template <class ResT>
class ExtendedImageStreamRequester : public BaseImageStreamRequester {

public:

    explicit ExtendedImageStreamRequester(ExtendedImageStream<ResT>& im_stream);

    void start() override;

    void copyData(ImageResponse& out, ResT& processing_result);

private:

    ExtendedImageStream<ResT>& image_stream;
    ImageResponse image_response;
    ResT processing_result;

};


#endif
