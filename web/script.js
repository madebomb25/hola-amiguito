document.addEventListener("DOMContentLoaded", function() {

    const products = [
        {id: 1, name: "Aprovat en SIOP", time: {h: 2, m: 0}, img: "https://via.placeholder.com/150?text=Aprovat"},
        {id: 2, name: "Matrícula d'honor en SIOP", time: {h: 5, m: 30}, img: "https://via.placeholder.com/150?text=Matrícula+Honor"},
        {id: 3, name: "Bolígraf", time: {h: 100, m: 0}, img: "https://via.placeholder.com/150?text=Bolígraf"},
        {id: 4, name: "Apple MacTime M3", time: {h: 50, m: 0}, img: "https://via.placeholder.com/150?text=MacTime+M3"},
        {id: 5, name: "Glock 18", time: {h: 80, m: 0}, img: "https://via.placeholder.com/150?text=Glock+18"},
        {id: 6, name: "Ampolla d'aigua d'acer inoxidable", time: {h: 0, m: 45}, img: "https://via.placeholder.com/150?text=Ampolla"},
        {id: 7, name: "Francotirador Barret M82", time: {h: 120, m: 0}, img: "https://via.placeholder.com/150?text=Barret+M82"},
        {id: 8, name: "Yamaha WaveRunner VX23", time: {h: 200, m: 0}, img: "https://via.placeholder.com/150?text=WaveRunner"},
        {id: 9, name: "Glock 20", time: {h: 90, m: 0}, img: "https://via.placeholder.com/150?text=Glock+20"},
        {id: 10, name: "Suspés remot en SIOP", time: {h: 3, m: 0}, img: "https://via.placeholder.com/150?text=Suspés+Remot"},
        {id: 11, name: "Pastilles de cafeïna", time: {h: 0, m: 30}, img: "https://via.placeholder.com/150?text=Cafeïna"},
        {id: 12, name: "Rellotge bàsic", time: {h: 0, m: 15}, img: "https://via.placeholder.com/150?text=Rellotge+Bàsic"}
    ];

    let totalTime = JSON.parse(localStorage.getItem('cartTotalTime')) || {h:0, m:0};

    function displayProducts() {
        const grid = document.getElementById('productGrid');
        grid.innerHTML = "";
        products.forEach(product => {
            const div = document.createElement('div');
            div.classList.add('product');
            div.innerHTML = `
                <img src="${product.img}" alt="${product.name}">
                <h3>${product.name}</h3>
                <p>Cost: ${product.time.h}h ${product.time.m}m</p>
                <button onclick="addToCart(${product.id})">Compra amb Temps ⏳</button>
            `;
            grid.appendChild(div);
        });
    }

    window.addToCart = function(id) {
        const product = products.find(p => p.id === id);

        // Update total time
        totalTime.h += product.time.h;
        totalTime.m += product.time.m;
        if(totalTime.m >= 60) {
            totalTime.h += Math.floor(totalTime.m / 60);
            totalTime.m = totalTime.m % 60;
        }
        document.getElementById('totalTime').textContent = `${totalTime.h}h ${totalTime.m}m`;

        // Save total time
        localStorage.setItem('cartTotalTime', JSON.stringify(totalTime));

        // Save products in cart
        let cartProducts = JSON.parse(localStorage.getItem('cartProducts'));
        if(!Array.isArray(cartProducts)) cartProducts = [];
        cartProducts.push(product);
        localStorage.setItem('cartProducts', JSON.stringify(cartProducts));
    };

    window.goToCart = function() {
        window.location.href = "cart.html";
    }

    // Display current total time on load
    document.getElementById('totalTime').textContent = `${totalTime.h}h ${totalTime.m}m`;

    displayProducts();
});

