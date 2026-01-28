function drawRuler(ruler_obj, wnd_limit) {
    let i = 50;
    while (i <= wnd_limit) {
        let div = document.createElement('div');
        div.setAttribute('class', 'grad');

        if (i % 100 == 0) {
            let span = document.createElement('span');
            span.textContent = i;
            div.appendChild(span);
        }

        ruler_obj.append(div);
        i += 50
    }
}

function showViewportSize() {
    let tmp_width = window.innerWidth,
        tmp_height = window.innerHeight;

    // change the values from class "num"
    let node_lst = document.querySelectorAll(".num");
    let node_lst_size = node_lst.length;
    if (node_lst_size > 0) {
        node_lst[0].textContent = tmp_width;
        node_lst[node_lst_size - 1].textContent = tmp_height;
    }

    // clearance the ruler
    let divs = document.getElementsByClassName('grad');
    let size = divs.length;
    for (let i = size - 1; i >= 0; i--)
        divs[i].remove();

    // add ruler for width
    let scr_width = "width:" + tmp_width + "px";
    document.getElementById('horiz').setAttribute("style", scr_width);
    drawRuler(document.getElementById('horiz'), innerWidth);

    // add ruler for height
    let scr_height = "height:" + tmp_height + "px";
    document.getElementById('vert').setAttribute("style", scr_height);
    drawRuler(document.getElementById('vert'), innerHeight);
}

let vert_elem = document.querySelector("#vert");
vert_elem.innerHeight = window.innerHeight;

let horiz_elem = document.querySelector("#horiz");
horiz_elem.innerHeight = window.innerWidth;

showViewportSize();
window.addEventListener('resize', showViewportSize);
