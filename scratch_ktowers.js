const CAMPAIGN_STAGES = [
    { id: 1,  name: "Beginner's Stack", disks: 3, pegs: 3, timeLimit: 0,  moveLimit: 0,  adjOnly: false, cyclic: false, colorRestr: 0, lockedDisk: 0, lockDuration: 0, par: 7 },
    { id: 2,  name: "Step Up",          disks: 4, pegs: 3, timeLimit: 0,  moveLimit: 0,  adjOnly: false, cyclic: false, colorRestr: 0, lockedDisk: 0, lockDuration: 0, par: 15 },
    { id: 3,  name: "Linear Steps",      disks: 3, pegs: 3, timeLimit: 0,  moveLimit: 0,  adjOnly: true,  cyclic: false, colorRestr: 0, lockedDisk: 0, lockDuration: 0, par: 26 },
    { id: 4,  name: "Reve's Intro",      disks: 5, pegs: 4, timeLimit: 0,  moveLimit: 0,  adjOnly: false, cyclic: false, colorRestr: 0, lockedDisk: 0, lockDuration: 0, par: 13 },
    { id: 5,  name: "Sprint Trial",      disks: 4, pegs: 3, timeLimit: 40, moveLimit: 0,  adjOnly: false, cyclic: false, colorRestr: 0, lockedDisk: 0, lockDuration: 0, par: 15 },
    { id: 6,  name: "Move Efficiency",  disks: 4, pegs: 3, timeLimit: 0,  moveLimit: 20, adjOnly: false, cyclic: false, colorRestr: 0, lockedDisk: 0, lockDuration: 0, par: 15 },
    { id: 7,  name: "Quad Towers",       disks: 6, pegs: 4, timeLimit: 0,  moveLimit: 0,  adjOnly: false, cyclic: false, colorRestr: 0, lockedDisk: 0, lockDuration: 0, par: 17 },
    { id: 8,  name: "Locked Foundation", disks: 4, pegs: 3, timeLimit: 0,  moveLimit: 0,  adjOnly: false, cyclic: false, colorRestr: 0, lockedDisk: 4, lockDuration: 5, par: 15 },
    { id: 9,  name: "Penta Realm",       disks: 7, pegs: 5, timeLimit: 0,  moveLimit: 0,  adjOnly: false, cyclic: false, colorRestr: 0, lockedDisk: 0, lockDuration: 0, par: 19 },
    { id: 10, name: "Chain Migration",   disks: 4, pegs: 4, timeLimit: 0,  moveLimit: 0,  adjOnly: true,  cyclic: false, colorRestr: 0, lockedDisk: 0, lockDuration: 0, par: 15 },
    { id: 11, name: "Clockwork Tower",   disks: 5, pegs: 3, timeLimit: 60, moveLimit: 0,  adjOnly: false, cyclic: false, colorRestr: 0, lockedDisk: 0, lockDuration: 0, par: 31 },
    { id: 12, name: "Cyclic Orbit",       disks: 4, pegs: 3, timeLimit: 0,  moveLimit: 0,  adjOnly: false, cyclic: true,  colorRestr: 0, lockedDisk: 0, lockDuration: 0, par: 25 },
    { id: 13, name: "Reve's Master",     disks: 8, pegs: 4, timeLimit: 0,  moveLimit: 0,  adjOnly: false, cyclic: false, colorRestr: 0, lockedDisk: 0, lockDuration: 0, par: 33 },
    { id: 14, name: "Spectrum Filter",    disks: 4, pegs: 4, timeLimit: 0,  moveLimit: 0,  adjOnly: false, cyclic: false, colorRestr: 1, lockedDisk: 0, lockDuration: 0, par: 17 },
    { id: 15, name: "Precision Stack",   disks: 5, pegs: 3, timeLimit: 0,  moveLimit: 38, adjOnly: false, cyclic: false, colorRestr: 0, lockedDisk: 0, lockDuration: 0, par: 31 },
    { id: 16, name: "Heavy Chains",      disks: 5, pegs: 4, timeLimit: 0,  moveLimit: 0,  adjOnly: true,  cyclic: false, colorRestr: 0, lockedDisk: 5, lockDuration: 8, par: 21 },
    { id: 17, name: "Cyclic Cascade",     disks: 5, pegs: 4, timeLimit: 0,  moveLimit: 0,  adjOnly: false, cyclic: true,  colorRestr: 0, lockedDisk: 0, lockDuration: 0, par: 35 },
    { id: 18, name: "Chromatic Citadel",  disks: 6, pegs: 4, timeLimit: 90, moveLimit: 0,  adjOnly: false, cyclic: false, colorRestr: 1, lockedDisk: 0, lockDuration: 0, par: 25 },
    { id: 19, name: "Pentagonal Matrix",  disks: 9, pegs: 5, timeLimit: 0,  moveLimit: 0,  adjOnly: false, cyclic: false, colorRestr: 0, lockedDisk: 0, lockDuration: 0, par: 25 },
    { id: 20, name: "Tower Grandmaster", disks: 10,pegs: 5, timeLimit: 0,  moveLimit: 0,  adjOnly: false, cyclic: false, colorRestr: 0, lockedDisk: 0, lockDuration: 0, par: 31 }
];

function bfsSolver(stage) {
    const numPegs = stage.pegs;
    const numDiscs = stage.disks;
    const targetPeg = numPegs - 1;
    
    let startPegs = Array.from({length: numPegs}, () => []);
    for (let i = numDiscs; i >= 1; i--) startPegs[0].push(i);
    
    let queue = [{
        pegs: startPegs,
        moves: 0,
        history: [] // store a few to debug if needed
    }];
    let visited = new Set();
    visited.add(JSON.stringify(startPegs));
    
    let head = 0;
    while(head < queue.length) {
        let curr = queue[head++];
        if (curr.pegs[targetPeg].length === numDiscs) {
            return curr.moves;
        }
        
        for (let f = 0; f < numPegs; f++) {
            if (curr.pegs[f].length === 0) continue;
            let topDisc = curr.pegs[f][curr.pegs[f].length - 1];
            
            if (stage.lockedDisk > 0 && topDisc === stage.lockedDisk && curr.moves < stage.lockDuration) {
                continue;
            }
            
            for (let t = 0; t < numPegs; t++) {
                if (f === t) continue;
                if (stage.adjOnly && Math.abs(f - t) !== 1) continue;
                if (stage.cyclic && ((f + 1) % numPegs !== t)) continue;
                if (stage.colorRestr === 1 && t === 1 && (topDisc % 2 !== 0)) continue;
                
                if (curr.pegs[t].length === 0 || curr.pegs[t][curr.pegs[t].length - 1] > topDisc) {
                    let nextPegs = curr.pegs.map(p => [...p]);
                    nextPegs[t].push(nextPegs[f].pop());
                    let stateStr = JSON.stringify(nextPegs);
                    if (!visited.has(stateStr)) {
                        visited.add(stateStr);
                        queue.push({
                            pegs: nextPegs,
                            moves: curr.moves + 1
                        });
                    }
                }
            }
        }
    }
    return -1;
}

CAMPAIGN_STAGES.forEach(s => {
    let opt = bfsSolver(s);
    console.log(`Stage ${s.id}: ${s.name} - Calculated Optimal: ${opt}, Config Par: ${s.par}`);
});
