document.addEventListener("DOMContentLoaded", () => {
    const CART_KEY = "cartItems";

    const cartList = document.getElementById("cartList");
    const cartTotalEl = document.getElementById("cartTotal");
    const cartItemsEl = document.getElementById("cartItems");
    const checkoutButton = document.getElementById("checkoutButton");
    const checkoutButtonGhost = document.getElementById("checkoutButtonGhost");
    const clearCartBtn = document.getElementById("clearCart");
    const paymentModal = document.getElementById("paymentModal");
    const closeModalBtn = document.getElementById("closeModal");
    const paymentForm = document.getElementById("paymentForm");
    const successPopup = document.getElementById("successPopup");
    const returnHomeBtn = document.getElementById("returnHome");
    const canvas = document.getElementById("particleCanvas");
    const cardNumberInput = document.getElementById("cardNumber");
    const expiryInput = document.getElementById("cardExpiry");
    const cvvInput = document.getElementById("cardCvv");

    const products = [
<<<<<<< HEAD
        { id: 1, name: "Aprovat en SIOP", timeMinutes: 120, img: "images/Aprovat en SIOP.png" },
        { id: 2, name: "Matrícula d'honor en SIOP", timeMinutes: 330, img: "images/Matrícula d'honor en SIOP.png" },
        { id: 3, name: "Bolígraf Arcà", timeMinutes: 100 * 60, img: "images/boligraf.png" },
        { id: 4, name: "Apple MacTime M3", timeMinutes: 3000, img: "images/Apple MacTime M3.png" },
        { id: 5, name: "Glock 18", timeMinutes: 4800, img: "images/Glock 18.png" },
        { id: 6, name: "Ampolla d'acer encantada", timeMinutes: 45, img: "images/Ampolla d'aigua d'acer inoxidable.png" },
        { id: 7, name: "Francotirador Barret M82", timeMinutes: 7200, img: "images/Francotirador Barret M82.png" },
        { id: 8, name: "Yamaha WaveRunner VX23", timeMinutes: 12000, img: "images/Yamaha WaveRunner VX23.png" },
        { id: 9, name: "Glock 20", timeMinutes: 5400, img: "images/Glock 20.png" },
        { id: 10, name: "Suspés remot en SIOP", timeMinutes: 180, img: "images/Suspés remot en SIOP.png" },
        { id: 11, name: "Pastilles de cafeïna", timeMinutes: 30, img: "images/Pastilles de cafeïna.png" },
        { id: 12, name: "Rellotge bàsic", timeMinutes: 15, img: "images/Rellotge bàsic.png" },
        { id: 13, name: "Drac del Vòrtex", timeMinutes: 9600, img: "images/dragon.png" },
        { id: 14, name: "Poció d'Amor Temporal", timeMinutes: 75, img: "images/poscima del amor.png" },
=======
        { id: 1, name: "Aprovat en SIOP", timeMinutes: 120, img: "images/AprovatenSIOP.png" },
        { id: 2, name: "Matrícula d'honor en SIOP", timeMinutes: 330, img: "images/MatriculadhonorenSIOP.png" },
        { id: 3, name: "Bolígraf Arcà", timeMinutes: 100 * 60, img: "images/boligraf.png" },
        { id: 4, name: "Apple MacTime M3", timeMinutes: 3000, img: "images/AppleMacTimeM3.png" },
        { id: 5, name: "Glock 18", timeMinutes: 4800, img: "images/Glock18.png" },
        { id: 6, name: "Ampolla d'acer encantada", timeMinutes: 45, img: "images/Ampolladaiguadacerinoxidable.png" },
        { id: 7, name: "Francotirador Barret M82", timeMinutes: 7200, img: "images/FrancotiradorBarretM82.png" },
        { id: 8, name: "Yamaha WaveRunner VX23", timeMinutes: 12000, img: "images/YamahaWaveRunnerVX23.png" },
        { id: 9, name: "Glock 20", timeMinutes: 5400, img: "images/Glock20.png" },
        { id: 10, name: "Suspés remot en SIOP", timeMinutes: 180, img: "images/SuspesremotenSIOP.png" },
        { id: 11, name: "Pastilles de cafeïna", timeMinutes: 30, img: "images/Pastillesdecafeïna.png" },
        { id: 12, name: "Rellotge bàsic", timeMinutes: 15, img: "images/Rellotgebàsic.png" },
        { id: 13, name: "Drac del Vòrtex", timeMinutes: 9600, img: "images/dragon.png" },
        { id: 14, name: "Poció d'Amor Temporal", timeMinutes: 75, img: "images/poscimadelamor.png" },
>>>>>>> origin/hola_amiguita
        { id: 15, name: "Orbe Gemini QH9", timeMinutes: 420, img: "images/Gemini_Generated_Image_qh9c21qh9c21qh9c.png" }
    ];

    let cart = loadCart();

    function loadCart() {
        const stored = JSON.parse(localStorage.getItem(CART_KEY));
        if (Array.isArray(stored)) return stored;
        const legacy = JSON.parse(localStorage.getItem("cartProducts"));
        if (Array.isArray(legacy) && legacy.length) {
            const normalized = [];
            legacy.forEach((legacyItem) => {
                const product = products.find((p) => p.name === legacyItem.name);
                if (!product) return;
                const found = normalized.find((item) => item.id === product.id);
                if (found) found.quantity += 1;
                else normalized.push({ id: product.id, quantity: 1 });
            });
            localStorage.setItem(CART_KEY, JSON.stringify(normalized));
            localStorage.removeItem("cartProducts");
            localStorage.removeItem("cartTotalTime");
            return normalized;
        }
        return [];
    }

    function saveCart() {
        localStorage.setItem(CART_KEY, JSON.stringify(cart));
        const minutes = calculateCartMinutes();
        const { h, m } = splitTime(minutes);
        localStorage.setItem("cartTotalTime", JSON.stringify({ h, m }));
        updateSummary();
    }

    function splitTime(minutes) {
        return { h: Math.floor(minutes / 60), m: minutes % 60 };
    }

    function formatTime(minutes) {
        const { h, m } = splitTime(minutes);
        return `${h}h ${m}m`;
    }

    function calculateCartMinutes() {
        return cart.reduce((total, item) => {
            const product = products.find((p) => p.id === item.id);
            return product ? total + product.timeMinutes * item.quantity : total;
        }, 0);
    }

    function renderCart() {
        if (!cartList) return;

        cartList.innerHTML = "";
        if (!cart.length) {
            const empty = document.createElement("div");
            empty.className = "empty-state";
            empty.innerHTML = `<p>El carret està buit. Explora la botiga i sacrifica minuts.</p>`;
            cartList.appendChild(empty);
            setCheckoutState(false);
            return;
        }

        const fragment = document.createDocumentFragment();
        cart.forEach((item) => {
            const product = products.find((p) => p.id === item.id);
            if (!product) return;
            fragment.appendChild(createCartItem(product, item.quantity));
        });
        cartList.appendChild(fragment);
        setCheckoutState(true);
    }

    function createCartItem(product, quantity) {
        const wrapper = document.createElement("article");
        wrapper.className = "cart-item";
        wrapper.innerHTML = `
            <img src="${product.img}" alt="${product.name}">
            <div class="cart-item-info">
                <h3>${product.name}</h3>
                <p>Per unitat: ${formatTime(product.timeMinutes)}</p>
                <div class="quantity-controls">
                    <button data-action="decrease" data-id="${product.id}">-</button>
                    <span>${quantity}</span>
                    <button data-action="increase" data-id="${product.id}">+</button>
                </div>
            </div>
            <div class="cart-item-total">
                <span>Total</span>
                <strong>${formatTime(product.timeMinutes * quantity)}</strong>
                <button data-action="remove" data-id="${product.id}">Eliminar</button>
            </div>
        `;
        return wrapper;
    }

    function setCheckoutState(enabled) {
        if (checkoutButton) checkoutButton.disabled = !enabled;
        if (checkoutButtonGhost) checkoutButtonGhost.disabled = !enabled;
    }

    function updateSummary() {
        const minutes = calculateCartMinutes();
        if (cartTotalEl) cartTotalEl.textContent = formatTime(minutes);
        const count = cart.reduce((acc, item) => acc + item.quantity, 0);
        if (cartItemsEl) cartItemsEl.textContent = count;
    }

    function modifyQuantity(id, delta) {
        const entry = cart.find((item) => item.id === id);
        if (!entry) return;
        entry.quantity += delta;
        if (entry.quantity <= 0) {
            cart = cart.filter((item) => item.id !== id);
        }
        saveCart();
        renderCart();
    }

    function removeItem(id) {
        cart = cart.filter((item) => item.id !== id);
        saveCart();
        renderCart();
    }

    function clearCart() {
        cart = [];
        saveCart();
        renderCart();
    }

    function openPaymentModal() {
        if (!paymentModal) return;
        paymentModal.classList.add("open");
        paymentModal.setAttribute("aria-hidden", "false");
    }

    function closePaymentModal() {
        if (!paymentModal) return;
        paymentModal.classList.remove("open");
        paymentModal.setAttribute("aria-hidden", "true");
    }

    function showSuccessPopup() {
        if (!successPopup) return;
        successPopup.classList.add("visible");
        successPopup.setAttribute("aria-hidden", "false");
        setTimeout(() => {
            window.location.href = "index.html";
        }, 3500);
    }

    function validateExpiry(value) {
        if (!/^\d{2}\/\d{2}$/.test(value)) return false;
        const [monthStr, yearStr] = value.split("/");
        const month = Number(monthStr);
        const year = Number(yearStr);
        if (!Number.isFinite(month) || !Number.isFinite(year)) return false;
        if (month < 1 || month > 12) return false;

        const current = new Date();
        const fullYear = 2000 + year;
        const currentYear = current.getFullYear();
        const currentMonth = current.getMonth() + 1;

        if (fullYear < currentYear) return false;
        if (fullYear === currentYear && month < currentMonth) return false;
        return true;
    }

    cartList?.addEventListener("click", (event) => {
        const button = event.target.closest("button[data-action]");
        if (!button) return;
        const id = Number(button.dataset.id);
        const { action } = button.dataset;
        if (action === "increase") modifyQuantity(id, 1);
        else if (action === "decrease") modifyQuantity(id, -1);
        else if (action === "remove") removeItem(id);
    });

    clearCartBtn?.addEventListener("click", clearCart);

    checkoutButton?.addEventListener("click", openPaymentModal);
    checkoutButtonGhost?.addEventListener("click", openPaymentModal);
    closeModalBtn?.addEventListener("click", closePaymentModal);

    paymentModal?.addEventListener("click", (event) => {
        if (event.target === paymentModal) closePaymentModal();
    });

    cardNumberInput?.addEventListener("input", () => {
        const cleaned = cardNumberInput.value.replace(/\D/g, "").slice(0, 16);
        const grouped = cleaned.match(/.{1,4}/g) || [];
        cardNumberInput.value = grouped.join(" ");
    });

    expiryInput?.addEventListener("input", () => {
        let value = expiryInput.value.replace(/\D/g, "").slice(0, 4);
        if (value.length >= 3) value = `${value.slice(0, 2)}/${value.slice(2)}`;
        expiryInput.value = value;
    });

    cvvInput?.addEventListener("input", () => {
        cvvInput.value = cvvInput.value.replace(/\D/g, "").slice(0, 4);
    });

    paymentForm?.addEventListener("submit", (event) => {
        event.preventDefault();
        if (!cardNumberInput || !expiryInput || !cvvInput) return;
        const numberClean = cardNumberInput.value.replace(/\s/g, "");
        if (numberClean.length < 12) {
            cardNumberInput.setCustomValidity("Introdueix una targeta vàlida.");
            cardNumberInput.reportValidity();
            return;
        }
        cardNumberInput.setCustomValidity("");

        if (!validateExpiry(expiryInput.value)) {
            expiryInput.setCustomValidity("Caducitat incorrecta.");
            expiryInput.reportValidity();
            return;
        }
        expiryInput.setCustomValidity("");

        if (cvvInput.value.length < 3) {
            cvvInput.setCustomValidity("CVV incorrecte.");
            cvvInput.reportValidity();
            return;
        }
        cvvInput.setCustomValidity("");

        paymentForm.reset();
        closePaymentModal();
        clearCart();
        showSuccessPopup();
    });

    returnHomeBtn?.addEventListener("click", () => {
        window.location.href = "index.html";
    });

    window.goBack = function goBack() {
        window.location.href = "index.html";
    };

    function initParticles(target) {
        if (!target) return;
        const ctx = target.getContext("2d");
        let width = 0;
        let height = 0;
        const particles = Array.from({ length: 65 }, () => createParticle());

        function resize() {
            width = target.width = window.innerWidth;
            height = target.height = Math.max(
                document.body.scrollHeight,
                document.documentElement.scrollHeight,
                window.innerHeight
            );
        }

        function createParticle() {
            return {
                x: Math.random() * width,
                y: Math.random() * height,
                radius: Math.random() * 2 + 0.5,
                speedX: (Math.random() - 0.5) * 0.25,
                speedY: Math.random() * 0.5 + 0.1,
                alpha: Math.random() * 0.5 + 0.2,
                hue: 220 + Math.random() * 110
            };
        }

        function draw() {
            const neededHeight = Math.max(
                document.body.scrollHeight,
                document.documentElement.scrollHeight,
                window.innerHeight
            );
            if (neededHeight !== height) {
                height = neededHeight;
                target.height = height;
            }
            ctx.clearRect(0, 0, width, height);
            ctx.globalCompositeOperation = "lighter";
            particles.forEach((particle) => {
                particle.x += particle.speedX;
                particle.y += particle.speedY;
                if (particle.y > height || particle.x < 0 || particle.x > width) {
                    Object.assign(particle, createParticle(), { y: -10 });
                }
                const gradient = ctx.createRadialGradient(
                    particle.x,
                    particle.y,
                    0,
                    particle.x,
                    particle.y,
                    particle.radius * 10
                );
                gradient.addColorStop(0, `hsla(${particle.hue}, 80%, 70%, ${particle.alpha})`);
                gradient.addColorStop(1, "transparent");
                ctx.fillStyle = gradient;
                ctx.beginPath();
                ctx.arc(particle.x, particle.y, particle.radius * 12, 0, Math.PI * 2);
                ctx.fill();
            });
            requestAnimationFrame(draw);
        }

        resize();
        window.addEventListener("resize", resize);
        draw();
    }

    initParticles(canvas);
    renderCart();
    updateSummary();
});

