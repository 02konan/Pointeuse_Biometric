async function loadDashboardData() {
    try {
        const response = await fetch('/api/dashboard_admin');
        
        if (!response.ok) {
            throw new Error('Erreur lors du chargement des données');
        }
        
        const result = await response.json();
        
        if (!result.success) {
            throw new Error(result.error || 'Erreur inconnue');
        }
        
        // Mise à jour des statistiques
        updateStatistics(result);
        
        // Mise à jour de l'activité récente
        updateRecentActivity(result.activite_recentes);
        
        // Mise à jour des graphiques
        updateCharts(result);
        
    } catch (error) {
        console.error('Erreur:', error);
        showErrorNotification('Impossible de charger les données du dashboard');
    }
}

// Fonction pour mettre à jour les statistiques
function updateStatistics(data) {
    const totalEmployes = data.total_employes || 0;
    
    // Statistiques du jour
    const totalPresents = data.total_presents || 0;
    const totalAbsents = data.total_absents || 0;
    const totalRetard = data.total_retard || 0;
    
    // Statistiques du mois
    const employesActifsMois = data.employes_actifs_mois || 0;
    const joursTravillesMois = data.jours_travailles_mois || 0;
    const employesRetardMois = data.employes_retard_mois || 0;
    const absencesMois = data.employes_retard_mois;
    
    // Mise à jour des compteurs du jour
    animateCounter('dash_presents-count', totalPresents);
    animateCounter('dash_absents-count', totalAbsents);
    animateCounter('dash_retard-count', totalRetard);
    animateCounter('dash_total-eleves', totalEmployes);
    
    // Mise à jour des compteurs du mois
    animateCounter('dash_presence-mois-count', employesActifsMois);
    animateCounter('dash_absence-mois-count', absencesMois >= 0 ? absencesMois : 0);
    animateCounter('dash_retard-mois-count', employesRetardMois);
    
    // Mise à jour des barres de progression (aujourd'hui)
    const presentPercentage = totalEmployes > 0 ? (totalPresents / totalEmployes) * 100 : 0;
    const absentPercentage = totalEmployes > 0 ? (totalAbsents / totalEmployes) * 100 : 0;
    const retardPercentage = totalEmployes > 0 ? (totalRetard / totalEmployes) * 100 : 0;
    
    updateProgressBar('dash_bar-present', presentPercentage);
    updateProgressBar('dash_bar-absent', absentPercentage);
    updateProgressBar('dash_bar-retard', retardPercentage);
    updateProgressBar('dash_total-eleves-bar', 100);
    
    // Mise à jour des barres de progression (mois)
    const maxPresencesMois = joursTravillesMois * totalEmployes;
    const presenceMoisPercentage = maxPresencesMois > 0 ? (employesActifsMois / maxPresencesMois) * 100 : 0;
    const absenceMoisPercentage = maxPresencesMois > 0 ? (absencesMois / maxPresencesMois) * 100 : 0;
    const retardMoisPercentage = maxPresencesMois > 0 ? (employesRetardMois / maxPresencesMois) * 100 : 0;
    
    updateProgressBar('dash_bar-presence-mois', presenceMoisPercentage);
    updateProgressBar('dash_bar-absence-mois', absenceMoisPercentage);
    updateProgressBar('dash_bar-retard-mois', retardMoisPercentage);
}

// Fonction pour animer les compteurs
function animateCounter(elementId, targetValue) {
    const element = document.getElementById(elementId);
    if (!element) return;
    
    const duration = 1000; // 1 seconde
    const startValue = parseInt(element.textContent) || 0;
    const startTime = performance.now();
    
    function update(currentTime) {
        const elapsed = currentTime - startTime;
        const progress = Math.min(elapsed / duration, 1);
        
        const currentValue = Math.floor(startValue + (progress * (targetValue - startValue)));
        element.textContent = currentValue;
        
        if (progress < 1) {
            requestAnimationFrame(update);
        } else {
            element.textContent = targetValue;
        }
    }
    
    requestAnimationFrame(update);
}

// Fonction pour mettre à jour les barres de progression
function updateProgressBar(elementId, percentage) {
    const element = document.getElementById(elementId);
    if (!element) return;
    
    const clampedPercentage = Math.min(Math.max(percentage, 0), 100);
    element.style.width = clampedPercentage + '%';
    element.setAttribute('aria-valuenow', clampedPercentage.toFixed(0));
}

// Fonction pour mettre à jour l'activité récente
function updateRecentActivity(activities) {
    const activityList = document.getElementById('dash_recent-activity-list');
    if (!activityList) return;
    
    activityList.innerHTML = '';
    
    if (!activities || activities.length === 0) {
        activityList.innerHTML = `
            <li class="list-group-item text-center text-muted py-4">
                <i class="fas fa-inbox fa-2x mb-2"></i>
                <p class="mb-0">Aucune activité récente</p>
            </li>
        `;
        return;
    }
    
    activities.forEach(activity => {
        const li = document.createElement('li');
        li.className = 'list-group-item';
        
        let icon = 'fa-circle';
        let iconColor = 'text-secondary';
        let statusBadgeClass = 'bg-secondary';
        
        const status = activity.status;

        
        if (status.includes('Arrivée enregistrée')) {
            icon = 'fa-check-circle';
            iconColor = 'text-success';
            statusBadgeClass = 'bg-success';
        }else{
            icon = 'fa-times-circle';
            iconColor = 'text-danger';
            statusBadgeClass = 'bg-danger';
        } 
        
        let dateFormatee = '';
        let heureFormatee = '';
        if (activity.date_pointage) {
            const date = new Date(activity.date_pointage);
            dateFormatee = date.toLocaleDateString('fr-FR', { day: '2-digit', month: 'short' });
            heureFormatee = date.toLocaleTimeString('fr-FR', { hour: '2-digit', minute: '2-digit' });
        }
        
        li.innerHTML = `
            <div class="d-flex align-items-start">
                <div class="me-3">
                    <i class="fas ${icon} ${iconColor} fa-lg"></i>
                </div>
                <div class="flex-grow-1">
                    <div class="d-flex justify-content-between align-items-start">
                        <div>
                            <strong>${activity.nom || 'Employé inconnu'}</strong>
                            <p class="mb-1 text-muted small">
                                <span class="badge ${statusBadgeClass}">${activity.status || 'Non défini'}</span>
                            </p>
                        </div>
                        <div class="text-end">
                            <small class="text-muted d-block">${dateFormatee}</small>
                            <small class="text-muted">${heureFormatee}</small>
                            <p class="mb-1 text-muted small">
                                <i class="fas fa-building me-1"></i>${activity.section || 'Section inconnue'}
                            </p>
                        </div>
                    </div>
                </div>
            </div>
        `;
        
        activityList.appendChild(li);
    });
}

// Variables pour les graphiques
let attendanceChart = null;
let attendancePieChart = null;

// Fonction pour mettre à jour les graphiques
function updateCharts(data) {
    updateAttendanceLineChart(data);
    updateAttendancePieChart(data);
}

// Graphique en ligne - Tendance de présence
function updateAttendanceLineChart(data) {
    const ctx = document.getElementById('dash_attendanceChart');
    if (!ctx) return;

    if (attendanceChart) {
        attendanceChart.destroy();
    }

    // Utiliser les labels et les données envoyées par Flask
    const labels =['Lun', 'Mar', 'Mer', 'Jeu', 'Ven', 'Sam','Dim'];
    const chartjs_Presents = data.chartjs_Presents || [];
    const chartjs_absents = data.chartjs_absents || [];
    const chartjs_retard = data.chartjs_retard || [];

    attendanceChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: [
                {
                    label: 'Présents',
                    data: chartjs_Presents,
                    borderColor: '#0d6efd',
                    backgroundColor: 'rgba(13, 110, 253, 0.1)',
                    tension: 0.4,
                    fill: true
                },
                {
                    label: 'Absents',
                    data: chartjs_absents,
                    borderColor: '#dc3545',
                    backgroundColor: 'rgba(220, 53, 69, 0.1)',
                    tension: 0.4,
                    fill: true
                },
                {
                    label: 'Retards',
                    data: chartjs_retard,
                    borderColor: '#ffc107',
                    backgroundColor: 'rgba(255, 193, 7, 0.1)',
                    tension: 0.4,
                    fill: true
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: { position: 'top' },
                tooltip: {
                    mode: 'index',
                    intersect: false,
                    callbacks: {
                        label: ctx => `${ctx.dataset.label}: ${ctx.parsed.y} employés`
                    }
                }
            },
            scales: { y: { beginAtZero: true, ticks: { stepSize: 1 } } }
        }
    });
}

// Fonction auxiliaire pour générer des données de tendance
function generateTrendData(currentValue, days) {
    const data = [];
    const variance = Math.max(2, Math.floor(currentValue * 0.15));
    
    for (let i = 0; i < days - 1; i++) {
        const randomVariance = Math.floor(Math.random() * variance * 2) - variance;
        data.push(Math.max(0, currentValue + randomVariance));
    }
    data.push(currentValue);
    
    return data;
}

// Graphique circulaire - Répartition des présences
function updateAttendancePieChart(data) {
    const ctx = document.getElementById('dash_attendancePieChart');
    if (!ctx) return;
    
    if (attendancePieChart) {
        attendancePieChart.destroy();
    }
    
    const totalPresents = data.total_presents || 0;
    const totalAbsents = data.total_absents || 0;
    const totalRetard = data.total_retard || 0;
    
    attendancePieChart = new Chart(ctx, {
        type: 'doughnut',
        data: {
            labels: ['Présents', 'Absents', 'Retards'],
            datasets: [{
                data: [totalPresents, totalAbsents, totalRetard],
                backgroundColor: ['#0d6efd','#dc3545','#ffc107'],
                borderWidth: 2,
                borderColor: '#fff'
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: {
                    position: 'bottom',
                    labels: { padding: 15, font: { size: 12 } }
                },
                tooltip: {
                    callbacks: {
                        label: function(context) {
                            const label = context.label || '';
                            const value = context.parsed || 0;
                            const total = context.dataset.data.reduce((a, b) => a + b, 0);
                            const percentage = total > 0 ? ((value / total) * 100).toFixed(1) : 0;
                            return `${label}: ${value} employés (${percentage}%)`;
                        }
                    }
                }
            }
        }
    });
}

// Fonction pour afficher une notification d'erreur
function showErrorNotification(message) {
    const alertDiv = document.createElement('div');
    alertDiv.className = 'alert alert-danger alert-dismissible fade show position-fixed top-0 end-0 m-3';
    alertDiv.style.zIndex = '9999';
    alertDiv.innerHTML = `
        <i class="fas fa-exclamation-circle me-2"></i>
        <strong>Erreur!</strong> ${message}
        <button type="button" class="btn-close" data-bs-dismiss="alert"></button>
    `;
    document.body.appendChild(alertDiv);
    setTimeout(() => alertDiv.remove(), 5000);
}

// Fonction pour rafraîchir le dashboard
function refreshDashboard() {
    loadDashboardData();
}

// Charger les données au chargement de la page
document.addEventListener('DOMContentLoaded', function() {
    loadDashboardData();
    setInterval(refreshDashboard, 5 * 60 * 1000);
});

// Événement sur le bouton "Voir tout"
document.addEventListener('DOMContentLoaded', function() {
    const voirToutBtn = document.querySelector('.card-header button');
    if (voirToutBtn) {
        voirToutBtn.addEventListener('click', function() {
            console.log('Voir toutes les activités');
            // window.location.href = '/historique-activites';
        });
    }
});
