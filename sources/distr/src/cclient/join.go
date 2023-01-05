package main

/*
 */
import "C"

import (
    "encoding/gob"
    "fmt"
    "log"
    "os"
    "dgeodistr/peer"
)

var (
    jopen_conns  map[string]chan peer.JoinMsg
    jpeers       map[byte]string
    jdispatchers chan peer.JoinEndMsg
    current_jm   peer.JoinMsg
)

//export NewJoin
func NewJoin(QueryName *C.char, Levels C.uchar) {
    jopen_conns = make(map[string]chan peer.JoinMsg)
    jpeers = make(map[byte]string)
    jdispatchers = make(chan peer.JoinEndMsg)
    current_jm = peer.JoinMsg{}

    req := peer.RequestMsg{}
    req.RType = peer.RJoin

    jsm := peer.JoinStartMsg{}
    jsm.QueryName = C.GoString(QueryName)
    jsm.Levels = uint8(Levels)

    for p, v := range aps {
        jpeers[v] = p

        enc, dec := connect(p)
        if enc != nil {
            chann := make(chan peer.JoinMsg, 5)
            jopen_conns[p] = chann
            go dispatchJoin(req, jsm, enc, dec, chann)
        }
    }
}

//export EndJoin
func EndJoin(Levels C.uchar) {
    for _, v := range jopen_conns {
        close(v)
    }
    results := make([]uint32, Levels)
    results_max := make([]uint32, Levels)
    results_min := make([]uint32, Levels)
    for i := 0; i < int(Levels); i++ {
        results_min[i] = ^uint32(0)
    }

    cpu_u := 0.0
    cpu_s := 0.0
    var net_in float64
    var net_out float64

    first := 0
    var cpu_u_min float64
    var cpu_u_max float64
    var cpu_s_min float64
    var cpu_s_max float64
    var cpu_t_min float64
    var cpu_t_max float64
    var net_in_min uint32
    var net_in_max uint32
    var net_out_min uint32
    var net_out_max uint32

    log.Printf("Join Stats Resume (CPU=ms, Net=Kb)\n")
    log.Printf("%20s | %10s | %10s | %10s | %10s | %10s | %s", "Host", "Cpu User", "Cpu Sys", "Cpu Tot", "Net In", "Net Out", "Results")
    fmtscr := "%20s | %10.1f | %10.1f | %10.1f | %10.1f | %10.1f | %6d"

    join_seconds := 0.0
    jems := make([]peer.JoinEndMsg, len(jpeers))
    for _, v := range jpeers {
        jem := <-jdispatchers
        jems = append(jems, jem)

        for i := 0; i < len(jem.ResultCardinality); i++ {
            results[i] += jem.ResultCardinality[i]
            if results_max[i] < jem.ResultCardinality[i] {
                results_max[i] = jem.ResultCardinality[i]
            }
            if results_min[i] > jem.ResultCardinality[i] {
                results_min[i] = jem.ResultCardinality[i]
            }
        }

        r := jem.Usage[len(jem.Usage)-1]
        cpu_u += r.User_cpu
        cpu_s += r.System_cpu
        net_in += float64(r.Net_in)
        net_out += float64(r.Net_out)
        log.Printf(fmtscr,
            v, r.User_cpu, r.System_cpu, r.User_cpu+r.System_cpu,
            float64(r.Net_in)/1024, float64(r.Net_out)/1024,
            jem.ResultCardinality)

        // Min/Max
        if first == 0 {
            cpu_u_min = r.User_cpu
            cpu_u_max = r.User_cpu
            cpu_s_min = r.System_cpu
            cpu_s_max = r.System_cpu
            cpu_t_min = r.User_cpu + r.System_cpu
            cpu_t_max = r.User_cpu + r.System_cpu
            net_in_min = r.Net_in
            net_in_max = r.Net_in
            net_out_min = r.Net_out
            net_out_max = r.Net_out
            first = 1
        } else {
            if cpu_u_min > r.User_cpu { cpu_u_min = r.User_cpu }
            if cpu_u_max < r.User_cpu { cpu_u_max = r.User_cpu }
            if cpu_s_min > r.System_cpu { cpu_s_min = r.System_cpu }
            if cpu_s_max < r.System_cpu { cpu_s_max = r.System_cpu }
            if cpu_t_min > r.User_cpu + r.System_cpu { cpu_t_min = r.User_cpu + r.System_cpu }
            if cpu_t_max < r.User_cpu + r.System_cpu { cpu_t_max = r.User_cpu + r.System_cpu }
            if net_in_min > r.Net_in { net_in_min = r.Net_in }
            if net_in_max < r.Net_in { net_in_max = r.Net_in }
            if net_out_min > r.Net_out { net_out_min = r.Net_out }
            if net_out_max < r.Net_out { net_out_max = r.Net_out }

        }

        // Print each server resource usage, over time
        fresusage, _ := os.OpenFile(fmt.Sprintf("resusage%s.txt", v), os.O_WRONLY|os.O_CREATE|os.O_TRUNC, 0644)
        logres := log.New(fresusage, "", 0)
        logres.Println("time\tUserCpu\tSysCpu\tTotCpu\tNetIn\tNetOut\tPredQ\tCopyQ")

        fmtru := "%.1f\t%.0f\t%.0f\t%.0f\t%.1f\t%.1f\t%d\t%d\n";
        priour := peer.ResourceUsage{jem.FirstUsage.AtTime, 0, 0, 0, 0, 0, 0}
        logres.Printf(fmtru, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0);
        for _, usage := range jem.Usage {
            second := usage.AtTime.Sub(jem.FirstUsage.AtTime).Seconds()
            if (join_seconds < second) {
                join_seconds = second
            }

            logres.Printf(fmtru,
                second,
                usage.User_cpu - priour.User_cpu,
                usage.System_cpu - priour.System_cpu,
                (usage.User_cpu - priour.User_cpu) + (usage.System_cpu - priour.System_cpu),
                float64(usage.Net_in - priour.Net_in)/1024,
                float64(usage.Net_out - priour.Net_out)/1024,
                usage.Pred_queue,
                usage.Copy_queue,
            )
            priour = usage;
        }
    }
    log.Printf(fmtscr,
        "Total", cpu_u, cpu_s, cpu_u+cpu_s,
        float64(net_in)/1024, float64(net_out)/1024, results)
    log.Printf(fmtscr,
        "Max", cpu_u_max, cpu_s_max, cpu_t_max,
        float64(net_in_max)/1024, float64(net_out_max)/1024, results_max)
    log.Printf(fmtscr,
        "Min", cpu_u_min, cpu_s_min, cpu_t_min,
        float64(net_in_min)/1024, float64(net_out_min)/1024, results_min)

    log.Printf("Join ended. Result cardinality: %d\n", results)

    // Non-blocking behavior
    steps := int(Levels)
    resultsmatrix := make([][]uint32, int(join_seconds)+1)
    for i := range resultsmatrix {
        resultsmatrix[i] = make([]uint32, steps)
    }

    for _, j := range jems {
        for s, sr := range j.StepsReturns {
            for _, inst := range sr.Instants {
                second := int(inst.AtTime.Sub(j.FirstUsage.AtTime).Seconds())
                resultsmatrix[second][s] += inst.Results
            }
        }
    }

    fmt.Fprintf(os.Stderr, "Results at each second\n")
    for i := range resultsmatrix {
        fmt.Fprintf(os.Stderr, "\n%d\t", i)
        for j := range resultsmatrix[i] {
            fmt.Fprintf(os.Stderr, "%10d\t", resultsmatrix[i][j])
        }
    }
    fmt.Fprintf(os.Stderr, "\n")

}

func dispatchJoin(req peer.RequestMsg, jsm peer.JoinStartMsg, enc *gob.Encoder, dec *gob.Decoder, jms chan peer.JoinMsg) {

    if !checkDispatch(enc.Encode(req)) {
        return
    }

    if !checkDispatch(enc.Encode(jsm)) {
        return
    }

    for jm := range jms {
        if !checkDispatch(enc.Encode(jm)) {
            return
        }
    }

    jm := peer.JoinMsg{}
    jm.End = true
    if !checkDispatch(enc.Encode(jm)) {
        return
    }

    jem := peer.JoinEndMsg{}
    err := dec.Decode(&jem)
    if err != nil {
        peer.Flog.Fatal(err)
    }

    jdispatchers <- jem
}

//export StartJoinMsg
func StartJoinMsg(lds *C.char, lx C.ushort, ly C.ushort, lserver C.int, level C.uchar) {
    current_jm = peer.JoinMsg{}
    current_jm.Level = uint8(level)
    current_jm.L = peer.DatasetSeg{}
    current_jm.L.Host = jpeers[byte(lserver)]
    current_jm.L.Dataset = C.GoString(lds)
    current_jm.L.Sid = peer.SegmentId{uint16(lx), uint16(ly)}
}

//export AddRToJoinMsg
func AddRToJoinMsg(rds *C.char, rx C.ushort, ry C.ushort, rserver C.int) {
    dsc := peer.DatasetSeg{}
    dsc.Host = jpeers[byte(rserver)]
    dsc.Dataset = C.GoString(rds)
    dsc.Sid = peer.SegmentId{uint16(rx), uint16(ry)}

    current_jm.R = append(current_jm.R, dsc)
}

//export EndJoinMsg
func EndJoinMsg(sendto C.int) {
    if len(current_jm.R) > 0 {
        jopen_conns[jpeers[byte(sendto)]] <- current_jm
    }
}
