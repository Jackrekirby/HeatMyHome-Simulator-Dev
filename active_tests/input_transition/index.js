console.log('javascript');

let box_id = 0;
const box_names = ['postcode-box', 'occupant-box', 'blah-blah'];

function next_box() {
    console.log('next boxes');
    if (false) {
        let boxes = document.getElementsByClassName('input-box');
        console.log(boxes);
        boxes[box_id].classList.add('hide');
        box_id++;
        boxes[box_id].classList.remove('hide');
    } else {
        let box_name = box_names[box_id];
        console.log('my current box is: ', box_name);
        document.getElementById(box_name).classList.add('hide');
        box_id++;
        box_name = box_names[box_id];
        console.log('my next box is: ', box_name);
        document.getElementById(box_name).classList.remove('hide');

        let fields = document.getElementById(box_name).getElementsByClassName('input-field');
        for (let field of fields) {
            field.classList.remove('fadein');
            field.classList.add('fadein');
        }
    }
}


function back_box() {
    if (box_id > 0) {
        console.log('back boxes');
        let boxes = document.getElementsByClassName('input-box');
        console.log(boxes);
        boxes[box_id].classList.add('hide');
        box_id--;
        boxes[box_id].classList.remove('hide');
    }
}

function done_box() {
    let postcode = document.getElementById('postcode');
    let ele = document.createElement('p');
    ele.textContent = 'Your postcode is: ' + postcode.value;
    document.body.appendChild(ele);
}


