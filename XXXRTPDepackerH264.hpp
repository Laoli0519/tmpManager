#ifndef XXXRTPDepackerH264_HPP
#define XXXRTPDepackerH264_HPP

#include "NMediaFrame.hpp"
#include "NLogger.hpp"
#include "NUtil.hpp"
#include "NRTPMap.hpp"


//+---------------+
//|0|1|2|3|4|5|6|7|
//+-+-+-+-+-+-+-+-+
//|F|NRI| Type    |
//+---------------+
class NH264Header : public NObjDumper::Dumpable {
public:
    struct NRTPayloadHeader {
        int8_t nal_unit_type_ = -1;
        int8_t nal_ref_idc_ = -1;
    };

    //FU-A header
    struct NNALUHeader {
        bool slice_start_sign_ = false;
        bool slice_end_sign_ = false;
        int8_t nal_unit_payload_type_ = -1;
    };

    //STAP-A Aggregation Packet (STAP)
    struct AggregationPacketHeader {
        uint16_t payload_size_;
        NRTPayloadHeader nal_header_;
    };

public:
    bool isKeyframe = false;
    NRTPayloadHeader rtp_payload_hdr_ = {0};

    //+---------------+---------------+
    //|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
    //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //|F|NRI| Type:28 |S|E|R| Type    |
    //+---------------+---------------+
    //FU-A
    //10 : first slice
    //01 : end slice
    //00 : intermediate slice
    NNALUHeader fu_a_header_ = {0};

    //0               1             2                 3
    //|0 1 2 3 4 5 6 7|8 9 0 1 2 3 4|5 6 7 8 9 0 1 2 3|4 5 6 7 8 9 0 1
    //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //|                          RTP Header                           |
    //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //|STAP-A NAL HDR |         NALU 1 Size           | NALU 1 HDR    |
    //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //|                         NALU 1 Data                           |
    //:                                                               :
    //+               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //|               | NALU 2 Size                   | NALU 2 HDR    |
    //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //|                         NALU 2 Data                           |
    //:                                                               :
    //|                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //|                               :...OPTIONAL RTP padding        |
    //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //STAP-A
    AggregationPacketHeader stap_a_header_;

public:
    virtual inline NObjDumper& dump(NObjDumper& dumper) const override{
        dumper.objB();
        dumper.kv("NALU type", rtp_payload_hdr_.nal_unit_type_);
        dumper.kv("nal_ref_idc", rtp_payload_hdr_.nal_ref_idc_);
        if (23 >= rtp_payload_hdr_.nal_unit_type_) {    //Signal NALU
            //todo ...
        } 
        else if ( 24 == rtp_payload_hdr_.nal_unit_type_) {  //STAP-A
            dumper.kv("stap_a_size", stap_a_header_.payload_size_);
            dumper.kv("stap_a_nal_ref_idc", stap_a_header_.nal_header_.nal_ref_idc_);
            dumper.kv("stap_a_nal_unit_type", stap_a_header_.nal_header_.nal_unit_type_);
        }
        else if ( 25 == rtp_payload_hdr_.nal_unit_type_) {

        }
        else if (28 == rtp_payload_hdr_.nal_unit_type_) {   //FU-A
            dumper.kv("fu_start_sign", fu_a_header_.slice_start_sign_);
            dumper.kv("fu_end_sign", fu_a_header_.slice_end_sign_);
            dumper.kv("fu_nal_type", fu_a_header_.nal_unit_payload_type_);
        }
        else if ( 29 == rtp_payload_hdr_.nal_unit_type_) {
            
        }

        dumper.objE();
        return dumper;
    } 

    uint32_t Parse(const uint8_t* data, uint32_t size) {
        if(size < 1){
            return 0;
        }

        uint32_t len = 0;

        if (0 == rtp_payload_hdr_.nal_unit_type_) {
            rtp_payload_hdr_.nal_ref_idc_ = data[len] >> 5 & 0x3;
            rtp_payload_hdr_.nal_unit_type_ = data[len] & 0x1F;
            ++len;
        }

        if(5 == rtp_payload_hdr_.nal_unit_type_) {
            isKeyframe = true;
        } else if(24 == rtp_payload_hdr_.nal_unit_type_) {
            if(size < len + 3) {
                return 0;
            }
            stap_a_header_.payload_size_ = (uint16_t)data[len] << 8 | (uint16_t)data[len + 1]; //big endian
            len += 2;
            stap_a_header_.nal_header_.nal_ref_idc_ = data[len] >> 5 & 0x3;
            stap_a_header_.nal_header_.nal_unit_type_ = data[len] & 0x1F;
            ++len;
        } else if (28 == rtp_payload_hdr_.nal_unit_type_) {
            if(size < len + 1) {
                return 0;
            }
            fu_a_header_.slice_start_sign_ = data[len] >> 7 & 0x1;
            fu_a_header_.slice_end_sign_ = data[len] >> 6 & 0x1;
            fu_a_header_.nal_unit_payload_type_ = data[len] & 0x1F;
            ++len;
        }

        return len;
    } 

    void Clear() {
        isKeyframe = false;
        memset(&rtp_payload_hdr_, 0x00, sizeof(NRTPayloadHeader));
        memset(&fu_a_header_, 0x00, sizeof(NNALUHeader));
        memset(&stap_a_header_, 0x00, sizeof(AggregationPacketHeader));
    }
};

class NH264Unit : public NVideoFrame {
public:
    class Pool : public NPool<NH264Unit, NMediaFrame>{
        
    };
public:
    NH264Unit() : NVideoFrame(NCodec::H264){ 
    }

    virtual bool isKeyframe() const override{
        return h264_header_.isKeyframe;
    }

   virtual const NVideoSize& videoSize() const override{
        return vide_size_;
    }

    virtual int64_t pictureId() const override {
        return -1;
    }

    void setHeader(const NH264Header& header) {
        h264_header_ = header;
    }

    const NH264Header& getHeader() const {
        return h264_header_;
    }

    void setTimestamp(const int64_t& timestamp) {
        timestamp_ = timestamp;
    }

    void setSequenceNumber(const uint16_t seq_num) {
        sequence_number_ = seq_num;
    }

    
private:
    NH264Header h264_header_;
    int64_t timestamp_ = -1;
    uint16_t sequence_number_ = 0;
    NVideoSize vide_size_;
};

#define NRTPDEPACK_NO_ERROR 0
#define NRTPDEPACK_COMPLETE 1
class NRTPDepacker : public NObjDumper::Dumpable{
public:
    DECLARE_CLASS_ENUM(ErrorCode,
                       kNoError = NRTPDEPACK_NO_ERROR,
                       kComplete = NRTPDEPACK_COMPLETE,
                       );
    typedef std::function<void(NMediaFrame::Unique& frame)> FrameFunc;
    
    NRTPDepacker() {
    }
    
    virtual ~NRTPDepacker(){
    }
    
    virtual const char * getErrorStr(int errorCode) const {
        return getNameForErrorCode((ErrorCode)errorCode).c_str();
    }
    
    virtual const NRTPSeq& nextSeq() const{
        return nextSeq_;
    }
    
    virtual int64_t firstTimeMS() const{
        return firstTimeMS_;
    }
    
    virtual int depack(const NRTPData& rtpd, bool add_payload, const FrameFunc& func) = 0;
    
    virtual int nextDecodable(const NRTPData& rtpd) = 0;
    
public:
    static NRTPDepacker* CreateDepacker(const NRTPCodec * codec);
    static NRTPDepacker* CreateDepacker(NCodec::Type typ);
    
protected:
    bool                startPhase_     = true;
    int64_t             firstTimeMS_     = 0;
    NRTPSeq             nextSeq_        = 0u;

};


class NRTPDepackH264 : public NRTPDepacker{
public:
    DECLARE_CLASS_ENUM(ErrorCode,
                       kLowPriority = -10,
                       kErrorTimestamp = -9,
                       kNotDecodable = -7,
                       kNotFirst = -6,
                       kSeqDisorder = -4,
                       kErrorHeaderSize = -3,
                       kZeroLength = -1,
                       kNoError = NRTPDEPACK_NO_ERROR,
                       kComplete = NRTPDEPACK_COMPLETE,
                       );
public:
    NRTPDepackH264(std::shared_ptr<NH264Unit::Pool>& pool)
        :NRTPDepacker(),
        pool_(pool),
        frame_(pool_->get()) {
        frame_->Clear();
    }
    
    virtual ~NRTPDepackH264() {
    }

    virtual const char * getErrorStr(int errorCode) const override{
        return getNameForErrorCode((ErrorCode)errorCode).c_str();
    }


    int parsePacket(const NRTPData& rtpd){
        h264_header_len_ = 0;
        
        // check lenght
        if (!rtpd.payloadLen){
            return kZeroLength;
        }
        
        if(rtpd.codecType == NCodec::H264){
            // decode payload descriptr
            h264_header_len_ = h264_header_.Parse(rtpd.payloadPtr+depack_payload_offset_, (uint32_t)rtpd.payloadLen-depack_payload_offset_);
            // check size
            if (!h264_header_len_ || rtpd.payloadLen < h264_header_len_){
                h264_header_len_ = 0;
                return kErrorHeaderSize;
            }
        }

        return kNoError;
    }

        // TODO: aaa add TLayer support
    virtual int nextDecodable(const NRTPData& rtpd) override{
        ErrorCode state = (ErrorCode) parsePacket(rtpd);
        if(state < 0) {
            return state;
        }

        state = (ErrorCode)continuousPacket(rtpd);
        if(state < 0){
            return state;
        }

        return kNoError;
    }

    int continuousSeq(const NRTPData& rtpd){
        return (startPhase_ || nextSeq_ == rtpd.header.sequenceNumber) ?
        kNoError : kSeqDisorder;
    }

    //make sure to remove RTP data padding;
    virtual int depack(const NRTPData& rtpd, bool add_payload, const FrameFunc& func) override{
        depack_payload_offset_ = 0;
        nextSeq_ = NRTPSeq(rtpd.header.sequenceNumber) + 1;
        ErrorCode state = kNoError;

        while(1) {
            state = (ErrorCode) nextDecodable(rtpd);
            if(state < 0) {
                nextSeq_ = nextSeq_ - 1;
                return state;
            }

            if(h264_header_len_ > 0) {
                depack_payload_offset_ += h264_header_len_ + h264_header_.stap_a_header_.payload_size_;
                if(add_payload) {
                    int nalu_payload_len = rtpd.payloadLen - h264_header_len_;
                    if (24 == h264_header_.rtp_payload_hdr_.nal_unit_type_) {
                        nalu_payload_len = h264_header_.stap_a_header_.payload_size_;
                    }
                    frame_->AppendData(rtpd.payloadPtr+depack_payload_offset_-h264_header_.stap_a_header_.payload_size_, nalu_payload_len);
                }
                if(first_packet_) {
                    firstTimeMS_ = rtpd.timeMS;
                    frame_->setPts(rtpd.header.timestamp);
                    frame_->setGts(rtpd.timeMS);
                    first_packet_ = false;
                }
            }

            if(rtpd.header.mark){
                first_packet_ = true;
                frame_->setCodecType(NCodec::H264, NMedia::Video);
                NH264Unit * h264_nalu = (NH264Unit *) frame_.get();
                h264_nalu->setHeader(this->h264_header_);
                h264_nalu->setTimestamp(rtpd.header.timestamp);
                h264_nalu->setSequenceNumber(rtpd.header.sequenceNumber);
                func(frame_);
                
                if(!frame_){
                    frame_ = pool_->get();
                }else{
                    frame_->Clear();
                }
            }

            if(24 != h264_header_.rtp_payload_hdr_.nal_unit_type_ || 
                depack_payload_offset_ >= rtpd.payloadLen) {
                break;
            }
        }

        h264_header_.Clear();
        state = rtpd.header.mark ? kComplete : kNoError;

        return state;
    }

    virtual inline NObjDumper& dump(NObjDumper& dumper) const override{
        dumper.objB();
        // dumper.kv("needkey", (unsigned)expectKeyFrame_);
        // if(descLen_ > 0){
        dumper.dump("desc", h264_header_);
        // }
        
        // if(headerLen_ > 0){
        //     dumper.dump("hdr", vp8header_);
        // }
        
        dumper.objE();
        return dumper;
    }

    int continuousPacket(const NRTPData& rtpd){
        if(rtpd.codecType != NCodec::H264){
            if(startPhase_){
                return kNotFirst;
            }
            return continuousSeq(rtpd);
        }
        
        if(frame_->getPts() != rtpd.header.timestamp || 
            24 == h264_header_.rtp_payload_hdr_.nal_unit_type_){
            if(!first_packet_){
                //return kErrorTimestamp;
                first_packet_ = true;
            }
            frame_->Clear();
        }
        
        if(first_packet_ && !h264_header_.fu_a_header_.slice_start_sign_){
            return kNotFirst;
        }

        return kNoError;
    }

    // void setPacktizationMod(const SDP&)

private:
    int8_t packtization_mode = -1;
    std::shared_ptr<NH264Unit::Pool> pool_;
    NMediaFrame::Unique frame_;
    NH264Header         h264_header_;
    size_t              h264_header_len_;
    bool                first_packet_    = true;

    int                 depack_payload_offset_ = 0;
};




#endif