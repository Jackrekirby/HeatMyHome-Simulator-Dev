const data = {
    datasets: [{
        label: 'EB+PV',
        data: [{
            x: -10,
            y: 0,
        }],
        pointStyle: 'circle',
        backgroundColor: 'rgb(255, 0, 0)'
    }, {
        label: 'ASHP',
        data: [{
            x: -15,
            y: 1,
        }],
        fill: false,
        borderColor: 'rgb(0, 255, 0)',
        borderWidth: 1,
        pointStyle: 'rectRot',
        pointRadius: 5,
        backgroundColor: 'rgba(0, 255, 0, 0.5)'
    }],
};

const config = {
    type: 'scatter',
    data: data,
    options: {
        scales: {
            x: {
                type: 'linear',
                position: 'bottom',
                scaleLabel: {
                    display: true,
                    labelString: 'Date'
                }
            },
            plugins: {
                legend: {
                    labels: {
                        usePointStyle: true,
                    },
                    position: 'bottom'
                }
            }
        }
    }
}

const myChart = new Chart(
    document.getElementById('myChart'),
    config
);


