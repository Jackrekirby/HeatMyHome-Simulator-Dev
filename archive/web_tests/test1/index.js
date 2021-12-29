
let sortby_index = 7;
function compare(a, b) {
    const i = sortby_index;
    if (sortby_index == 4) {
        return 0;
    }

    if (a[i] < b[i]) {
        return -1;
    }
    if (a[i] > b[i]) {
        return 1;
    }
    return 0;
}

function drawTable(output_json) {
    let simtable = document.getElementById('sim-table');

    let groupbyopt = document.getElementById('groupby');
    groupby_index = Number(groupbyopt.value);

    let sortbyopt = document.getElementById('sortby');
    sortby_index = Number(sortbyopt.value) + 4;

    simtable = document.getElementById('sim-table');
    while (document.getElementsByTagName('tr').length > 1) {
        simtable.removeChild(simtable.lastChild);
    }

    let speci = 0;
    switch (groupby_index) {
        case 0:
            speci = Array(21).keys();
            output_json.sort(compare);
            break;
        case 1:
            speci = [0, 7, 14];
            //speci = Array(21).keys();
            a = output_json.slice(0, 7).sort(compare);
            b = output_json.slice(7, 14).sort(compare);
            c = output_json.slice(14, 21).sort(compare);
            output_json = a.concat(b, c);
            break;
        default: // 2
            b = output_json;
            output_json = [];
            for (let ii of Array(7).keys()) {
                let a = [b[ii], b[ii + 7], b[ii + 14]];
                a.sort(compare);
                console.log(a);
                output_json.push(a[0]);
                output_json.push(a[1]);
                output_json.push(a[2]);
            }
            speci = [0, 3, 6, 9, 12, 15, 18];
        //part_json.sort(compare);
    }

    let collapseGroupsEle = document.getElementById('collapse-groups');
    let collapseGroups = collapseGroupsEle.checked;
    //console.log(collapseGroups);
    if (!collapseGroups) {
        speci = Array(21).keys();
    }
    // let part_json = [];
    // for (let ii of speci) {
    //     part_json.push(output_json[ii]);
    // };
    //console.log(output_json);

    for (let ii of speci) {
        //console.log(ii);
        let tr = document.createElement('tr');
        for (let value of output_json[ii]) {
            let td = document.createElement('td');
            td.innerHTML = value;
            tr.appendChild(td);
        }
        simtable.appendChild(tr);
    }
}

function test() {
    const output_str = '[["ERH", "None", 0, 0, 0.2, 373, 949, 6435, 924],["ERH", "PV", 14, 0, 0.1, 173, 3759, 6308, 516],["ERH", "FP", 0, 2, 0.2, 316, 3527, 8182, 775],["ERH", "ET", 0, 2, 0.2, 305, 3637, 8129, 747],["ERH", "FP+PV", 12, 2, 0.1, 168, 5896, 8372, 478],["ERH", "ET+PV", 12, 2, 0.1, 147, 6006, 8172, 424],["ERH", "PVT", 2, 2, 0.2, 300, 4957, 9366, 748],["ASHP", "None", 0, 0, 0.1, 145, 6238, 8369, 355],["ASHP", "PV", 14, 0, 0.1, -107, 9318, 7751, -23],["ASHP", "FP", 0, 2, 0.1, 128, 8815, 10703, 324],["ASHP", "ET", 0, 2, 0.1, 124, 8925, 10745, 316],["ASHP", "FP+PV", 12, 2, 0.1, -77, 11455, 10319, 7],["ASHP", "ET+PV", 12, 2, 0.1, -90, 11565, 10236, -10],["ASHP", "PVT", 2, 2, 0.1, 111, 10245, 11880, 295],["GSHP", "None", 0, 0, 0.1, 90, 7938, 9257, 221],["GSHP", "PV", 14, 0, 0.1, -198, 11018, 8104, -163],["GSHP", "FP", 0, 2, 0.1, 77, 10515, 11653, 202],["GSHP", "ET", 0, 2, 0.1, 74, 10625, 11717, 198],["GSHP", "FP+PV", 12, 2, 0.1, -164, 13155, 10746, -123],["GSHP", "ET+PV", 12, 2, 0.1, -173, 13265, 10720, -134],["GSHP", "PVT", 2, 2, 0.1, 60, 11945, 12833, 173]]'
    const output_json = JSON.parse(output_str);
    drawTable(output_json);
}

function updateGroupBy() {
    let groupbyopt = document.getElementById('groupby');
    groupby_index = Number(groupbyopt.value);
    if (groupby_index == 0) {
        let elements = document.getElementsByName('collapse-groups');
        for (let element of elements) {
            console.log(element);
            element.classList.add("hide");
        }
    } else {
        let elements = document.getElementsByName('collapse-groups');
        for (let element of elements) {
            console.log(element);
            element.classList.remove("hide");
        }
    }
    test();
}

function updateSortBy() {
    test();
}

function updateCollapseGroups() {
    test();
}

test();